#include "brkga_mp_ipr/brkga_mp_ipr.hpp"
#include "strip_packing/heuristics.hpp"
#include "strip_packing/io.hpp"

#include <fstream>
#include <strip_packing.hpp>

#include <random>
#include <vector>

using namespace strip_packing;

template <typename URBG> instance_t generate_instance(URBG&& rng) {
    std::uniform_int_distribution<size_t> size_dist(300, 500);
    std::uniform_real_distribution<dim_type> cap_dist(75, 100);
    std::uniform_real_distribution<dim_type> dim_dist(3, 10);
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

int main() {
    std::random_device rd;

    auto seed = rd();
    std::cout << "Random seed: " << seed << std::endl << std::endl;

    std::mt19937_64 rng(seed);

    auto instance = generate_instance(rng);
    {
        std::ofstream out("instance.yml");
        io::write_instance(out, instance);
    }

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "[Generated instance]" << std::endl;
    io::print_instance(instance);

    std::cout << "[Randomized first-fit decreasing weight heuristic solution]"
              << std::endl;
    auto first_fit_solution =
        heuristics::constructive::randomized_first_fit_decreasing_weight(
            instance, rng, std::normal_distribution<>(0.0, 3.0));
    io::print_solution(instance, first_fit_solution);
    std::cout << std::endl;

    io::render_solution(instance, first_fit_solution, "ffdw.png");

    std::cout << "[Randomized best-fit increasing height heuristic solution]"
              << std::endl;
    auto best_fit_solution =
        heuristics::constructive::randomized_best_fit_increasing_height(
            instance, rng, std::normal_distribution<>(0.0, 2.0));
    io::print_solution(instance, best_fit_solution);
    std::cout << std::endl;

    io::render_solution(instance, best_fit_solution, "bfih.png");

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

    auto brkga_solution = heuristics::improvement::brkga_mp_ipr(
        instance, initial)(rng, brkga_params, control_params, 24);

    io::print_solution(instance, brkga_solution);
    std::cout << std::endl;

    io::render_solution(instance, brkga_solution, "brkga.png");
}
