#include "brkga_mp_ipr/brkga_mp_ipr.hpp"
#include "strip_packing/defs.hpp"
#include "strip_packing/heuristics.hpp"
#include <strip_packing.hpp>

#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace strip_packing;

template <typename URBG> instance_t gen_instance(URBG&& rng) {
    std::uniform_int_distribution<size_t> size_dist(100, 300);
    std::uniform_real_distribution<dim_type> cap_dist(30, 100);
    std::uniform_real_distribution<dim_type> dim_dist(2, 10);
    std::uniform_real_distribution<cost_type> weight_dist(0, 10);

    size_t instance_size = size_dist(rng);
    instance_t instance;
    instance.recipient_length = cap_dist(rng);
    instance.rects.reserve(instance_size);
    for (size_t i = 0; i < instance_size; i++) {
        instance.rects.push_back(
            {dim_dist(rng), dim_dist(rng), weight_dist(rng)});
    }

    return instance;
}

void print_instance(instance_t instance) {
    std::cout << "Instance size: " << instance.rects.size() << std::endl;
    std::cout << "Recipient length: " << instance.recipient_length << std::endl;
    std::cout << "Rects: " << std::endl;
    for (const auto& rect : instance.rects) {
        std::cout << rect.length << "x" << rect.height << "(" << rect.weight
                  << ") ";
    }
    std::cout << std::endl << std::endl;
}

void print_solution(instance_t instance, solution_t solution) {
    std::cout << "Cost: " << instance.cost(solution) << std::endl;
    dim_type h = 0;
    for (size_t i = 0; i < solution.size(); i++) {
        std::cout << "(level " << std::right << std::setw(2) << i
                  << ", h = " << std::setw(4) << h << ") ";
        dim_type max_h = 0;
        dim_type L = 0;
        for (const auto& j : solution[i]) {
            L += instance.rects[j].length;
            std::cout << std::setw(3) << std::right << j << ":" << std::setw(3)
                      << std::left << instance.rects[j].weight << " ";
            max_h = std::max(max_h, instance.rects[j].height);
        }
        std::cout << " (L = " << L << ")";
        h += max_h;
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    std::random_device rd;

    auto seed = rd();
    std::cout << "Random seed: " << seed << std::endl << std::endl;

    std::mt19937_64 rng(seed);

    auto instance = gen_instance(rng);

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "[Generated instance]" << std::endl;
    print_instance(instance);

    std::cout << "[Randomized first-fit decreasing weight heuristic solution]"
              << std::endl;
    auto first_fit_solution =
        heuristics::constructive::randomized_first_fit_decreasing_weight(
            instance, rng, std::normal_distribution<>(0.0, 3.0));
    print_solution(instance, first_fit_solution);
    std::cout << std::endl;

    std::cout << "[Randomized best-fit increasing height heuristic solution]"
              << std::endl;
    auto volume_solution =
        heuristics::constructive::randomized_best_fit_increasing_height(
            instance, rng, std::normal_distribution<>(0.0, 2.0));
    print_solution(instance, volume_solution);
    std::cout << std::endl;

    std::cout << "[BRKGA]" << std::endl;
    std::vector<solution_t> initial;
    initial.reserve(2500);

    for (int i = 0; i < 1250; i++) {
        auto solution =
            heuristics::constructive::randomized_first_fit_decreasing_weight(
                instance, rng, std::normal_distribution<>(0.0, 5.0));
        initial.push_back(solution);
    }

    for (int i = 0; i < 1250; i++) {
        auto solution =
            heuristics::constructive::randomized_best_fit_increasing_height(
                instance, rng, std::normal_distribution<>(0.0, 3.0));
        initial.push_back(solution);
    }

    std::shuffle(initial.begin(), initial.end(), rng);

    auto [brkga_params, control_params] =
        BRKGA::readConfiguration("brkga.conf");

    auto brkga_solution = heuristics::improvement::brkga_mp_ipr_improve(
        instance, initial, rng, brkga_params, control_params, 24);

    print_solution(instance, brkga_solution);
}
