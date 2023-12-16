#include <cmath>
#include <cstddef>
#include <fstream>
#include <random>
#include <stdexcept>

#include <strip_packing.hpp>

#include <argparse/argparse.hpp>

#include <brkga_mp_ipr/brkga_mp_ipr.hpp>

using namespace strip_packing;

/*! Ponto de entrada. */
int main(int argc, char** argv) {
    argparse::ArgumentParser program("mc859-strip-packing-heuristics");

    program.add_argument("-s", "--seed")
        .metavar("N")
        .help("seed for the random number generator.")
        .scan<'u', unsigned>();

    program.add_argument("-o", "--output")
        .metavar("DIR")
        .default_value<std::string>(".")
        .help("output directory.");

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

    heuristics::runner::config conf = {
        .random_seed = seed,
        .brkga_enabled = !program.get<bool>("--no-brkga"),
        .brkga_config = program.get("--brkga-config"),
        .first_fit_samples = program.get<unsigned>("--first-fit"),
        .first_fit_random_deviations =
            program.get<double>("--first-fit-deviations"),
        .best_fit_samples = program.get<unsigned>("--best-fit"),
        .best_fit_random_deviations =
            program.get<double>("--best-fit-deviations"),
        .output = program.get("--output")};

    heuristics::runner(instance, conf).run();
}
