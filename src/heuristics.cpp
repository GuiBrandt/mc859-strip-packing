#include <cmath>
#include <cstddef>
#include <fstream>
#include <stdexcept>

#include <strip_packing.hpp>
#include <strip_packing/io.hpp>
#include <strip_packing/render.hpp>

#include <argparse/argparse.hpp>

#include <brkga_mp_ipr/brkga_mp_ipr.hpp>

using namespace strip_packing;

class heuristics_runner {
  public:
    struct config {
        size_t random_seed;
        bool brkga_enabled;
        std::string brkga_config;
        size_t first_fit_samples;
        double first_fit_random_deviations;
        size_t best_fit_samples;
        double best_fit_random_deviations;
    };

  private:
    const instance_t& m_instance;
    const config& m_config;

    double m_weight_stddev;
    double m_height_stddev;

    template <class URBG>
    solution_t run_first_fit(URBG&& rng, size_t samples,
                             std::vector<solution_t>& solutions) {
        std::normal_distribution<> noise(
            0.0, m_config.first_fit_random_deviations * m_weight_stddev);
        solution_t best;
        cost_type best_cost = -1;
        for (size_t i = 0; i < samples; i++) {
            auto solution = heuristics::constructive::
                randomized_first_fit_decreasing_density(m_instance, rng, noise);
            solutions.push_back(solution);

            cost_type cost = m_instance.cost(solution);
            if (best_cost < 0 || cost < best_cost) {
                best = solution;
                best_cost = cost;
            }
        }
        return best;
    }

    template <class URBG>
    solution_t run_best_fit(URBG&& rng, size_t samples,
                            std::vector<solution_t>& solutions) {
        std::normal_distribution<> noise(
            0.0, m_config.best_fit_random_deviations * m_height_stddev);
        solution_t best;
        cost_type best_cost = -1;
        for (size_t i = 0; i < samples; i++) {
            auto solution =
                heuristics::constructive::randomized_best_fit_increasing_height(
                    m_instance, rng, noise);
            solutions.push_back(solution);

            cost_type cost = m_instance.cost(solution);
            if (best_cost < 0 || cost < best_cost) {
                best = solution;
                best_cost = cost;
            }
        }
        return best;
    }

    template <class URBG>
    solution_t run_brkga(URBG&& rng, const BRKGA::BrkgaParams& brkga_params,
                         const BRKGA::ControlParams& control_params,
                         std::vector<solution_t>&& initial) {
        std::shuffle(initial.begin(), initial.end(), rng);
        return heuristics::improvement::brkga_mp_ipr(m_instance, initial)
            .run(rng, brkga_params, control_params, 24);
    }

  public:
    heuristics_runner(const instance_t& instance, const config& conf)
        : m_instance(instance), m_config(conf) {

        double acc_weight = 0.0;
        double acc_height = 0.0;
        for (auto& rect : instance.rects) {
            acc_weight += rect.weight;
            acc_height += rect.height;
        }

        double mean_weight = acc_weight / instance.rects.size();
        double mean_height = acc_height / instance.rects.size();

        acc_weight = 0.0;
        acc_height = 0.0;
        for (auto& rect : instance.rects) {
            acc_weight += std::pow(rect.weight - mean_weight, 2);
            acc_height += std::pow(rect.height - mean_height, 2);
        }

        m_weight_stddev = std::sqrt(acc_weight / instance.rects.size());
        m_height_stddev = std::sqrt(acc_height / instance.rects.size());

        std::cout << "Weight standard deviation: " << m_weight_stddev
                  << std::endl;
        std::cout << "Height standard deviation: " << m_height_stddev
                  << std::endl;
    }

    void run() {
        std::mt19937_64 rng(m_config.random_seed);

        std::cout << std::fixed << std::setprecision(3);
        io::print_instance(m_instance);

        std::vector<solution_t> initial;
        initial.reserve(1000);

        std::cout << "[Randomized first-fit decreasing weight/height ratio "
                     "heuristic solution]"
                  << std::endl;
        auto first_fit_solution = run_first_fit(rng, 500, initial);
        io::print_solution(m_instance, first_fit_solution);
        std::cout << std::endl;
        render::render_solution(m_instance, first_fit_solution,
                                "first-fit.png");

        std::cout
            << "[Randomized best-fit increasing height heuristic solution]"
            << std::endl;
        auto best_fit_solution = run_best_fit(rng, 500, initial);
        io::print_solution(m_instance, best_fit_solution);
        std::cout << std::endl;
        render::render_solution(m_instance, best_fit_solution, "best-fit.png");

        if (m_config.brkga_enabled) {
            std::cout << "[BRKGA]" << std::endl;
            auto [brkga_params, control_params] =
                BRKGA::readConfiguration(m_config.brkga_config);

            brkga_params.population_size =
                std::max(brkga_params.population_size,
                         unsigned(m_config.best_fit_samples +
                                  m_config.first_fit_samples) *
                             2 / brkga_params.num_independent_populations);

            auto brkga_solution = run_brkga(rng, brkga_params, control_params,
                                            std::move(initial));
            io::print_solution(m_instance, brkga_solution);
            std::cout << std::endl;
            render::render_solution(m_instance, brkga_solution, "brkga.png");
        }
    }
};

int main(int argc, char** argv) {
    argparse::ArgumentParser program("mc859-strip-packing-heuristics");

    program.add_argument("-s", "--seed")
        .metavar("N")
        .help("seed for the random number generator.")
        .scan<'u', unsigned>();

    program.add_argument("--no-brkga")
        .default_value(false)
        .implicit_value(true)
        .help("disable BRKGA improvement.")
        .nargs(0);

    program.add_argument("--brkga-config")
        .default_value<std::string>("brkga.conf")
        .metavar("FILE")
        .help("BRKGA configuration file.");

    program.add_argument("--first-fit")
        .default_value<unsigned>(500)
        .metavar("N")
        .help("number of random samples of the first fit heuristic.")
        .scan<'u', unsigned>();

    program.add_argument("--first-fit-deviations")
        .default_value<double>(.25)
        .metavar("N")
        .help("standard deviations to use for randomization of the first fit "
              "heuristic.")
        .scan<'g', double>();

    program.add_argument("--best-fit")
        .default_value<unsigned>(500)
        .metavar("N")
        .help("number of random samples of the best fit heuristic.")
        .scan<'u', unsigned>();

    program.add_argument("--best-fit-deviations")
        .default_value<double>(.25)
        .metavar("N")
        .help("standard deviations to use for randomization of the best fit "
              "heuristic.")
        .scan<'g', double>();

    program.add_argument("file").help("instance file name.");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    size_t seed;
    if (auto s = program.present<unsigned>("-s")) {
        seed = *s;
    } else {
        std::random_device rd;
        seed = rd();
    }

    instance_t instance;
    {
        auto filename = program.get("file");
        std::ifstream file(filename);
        instance = io::read_instance(file);
    }

    heuristics_runner::config conf = {
        .random_seed = seed,
        .brkga_enabled = !program.get<bool>("--no-brkga"),
        .brkga_config = program.get("--brkga-config"),
        .first_fit_samples = program.get<unsigned>("--first-fit"),
        .first_fit_random_deviations =
            program.get<double>("--first-fit-deviations"),
        .best_fit_samples = program.get<unsigned>("--best-fit"),
        .best_fit_random_deviations =
            program.get<double>("--best-fit-deviations"),
    };

    heuristics_runner(instance, conf).run();
}
