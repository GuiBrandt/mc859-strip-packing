#include <strip_packing.hpp>

#include <iomanip>
#include <iostream>
#include <random>

int main(int argc, char** argv) {
    using namespace strip_packing;

    std::random_device rd;
    std::mt19937_64 rng(rd());

    std::uniform_int_distribution<size_t> size_dist(100, 1000);
    std::uniform_real_distribution<dim_type> cap_dist(3, 30);
    std::uniform_real_distribution<dim_type> dim_dist(0.1, 2.5);
    std::uniform_real_distribution<cost_type> weight_dist(0, 10);

    std::cout << std::fixed << std::setprecision(3);

    size_t instance_size = size_dist(rng);
    instance_t instance;
    instance.recipient_length = cap_dist(rng);
    instance.rects.reserve(instance_size);
    for (size_t i = 0; i < instance_size; i++) {
        instance.rects.push_back(
            {dim_dist(rng), dim_dist(rng), weight_dist(rng)});
    }

    std::cout << "[Generated instance]" << std::endl;
    std::cout << "Instance size: " << instance_size << std::endl;
    std::cout << "Recipient length: " << instance.recipient_length << std::endl;
    std::cout << "Rects: " << std::endl;
    for (const auto& rect : instance.rects) {
        std::cout << rect.length << "x" << rect.height << "(" << rect.weight
                  << ") ";
    }
    std::cout << std::endl << std::endl;

    solution_t solution = heuristics::constructive::first_fit_decreasing_random(
        instance, rng, std::normal_distribution<>(0.0, 3.0));

    std::cout << "[Randomized first-fit decreasing heuristic solution]"
              << std::endl;
    std::cout << "Cost: " << instance.cost(solution) << std::endl;
    dim_type h = 0;
    for (size_t i = 0; i < solution.size(); i++) {
        std::cout << "(level " << std::right << std::setw(2) << i
                  << ", h = " << std::setw(4) << h << ") ";
        dim_type max_h = 0;
        for (const auto& j : solution[i]) {
            std::cout << std::setw(3) << std::right << j << ":" << std::setw(3)
                      << std::left << instance.rects[j].weight << " ";
            max_h = std::max(max_h, instance.rects[j].height);
        }
        h += max_h;
        std::cout << std::endl;
    }
}
