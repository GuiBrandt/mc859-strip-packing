#include <iostream>
#include <random>

#include <strip_packing/defs.hpp>
#include <strip_packing/io.hpp>

#include <argparse/argparse.hpp>

using namespace strip_packing;

struct gen_config {
    size_t instance_size;
    dim_type recipient_length;
    dim_type length_min, length_max;
    dim_type height_min, height_max;
    dim_type weight_min, weight_max;
};

template <typename URBG>
instance_t generate_instance(URBG&& rng, gen_config config) {
    std::uniform_int_distribution<dim_type> length_dist(config.length_min,
                                                        config.length_max);
    std::uniform_int_distribution<dim_type> height_dist(config.height_min,
                                                        config.height_max);
    std::uniform_int_distribution<dim_type> weight_dist(config.weight_min,
                                                        config.weight_max);

    instance_t instance;
    instance.recipient_length = config.recipient_length;
    instance.rects.reserve(config.instance_size);
    for (size_t i = 0; i < config.instance_size; i++) {
        instance.rects.push_back({.length = length_dist(rng),
                                  .height = height_dist(rng),
                                  .weight = cost_type(weight_dist(rng))});
    }

    return instance;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("mc859-strip-packing-gen-instances");

    program.add_argument("-s", "--seed")
        .metavar("N")
        .help("seed for the random number generator.")
        .scan<'u', unsigned>();

    program.add_argument("instance_size")
        .help("instance size.")
        .scan<'u', unsigned>();

    program.add_argument("recipient_length")
        .help("recipient length.")
        .scan<'u', unsigned>();

    program.add_argument("length_min")
        .help("minimum rectangle length.")
        .scan<'u', unsigned>();
    program.add_argument("length_max")
        .help("maximum rectangle length.")
        .scan<'u', unsigned>();

    program.add_argument("height_min")
        .help("minimum rectangle height.")
        .scan<'u', unsigned>();
    program.add_argument("height_max")
        .help("maximum rectangle height.")
        .scan<'u', unsigned>();

    program.add_argument("weight_min")
        .help("minimum rectangle weight.")
        .scan<'u', unsigned>();
    program.add_argument("weight_max")
        .help("maximum rectangle weight.")
        .scan<'u', unsigned>();

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

    std::mt19937_64 rng(seed);

    gen_config config = {
        .instance_size = program.get<unsigned>("instance_size"),
        .recipient_length = program.get<unsigned>("recipient_length"),
        .length_min = program.get<unsigned>("length_min"),
        .length_max = program.get<unsigned>("length_max"),
        .height_min = program.get<unsigned>("height_min"),
        .height_max = program.get<unsigned>("height_max"),
        .weight_min = program.get<unsigned>("weight_min"),
        .weight_max = program.get<unsigned>("weight_max"),
    };

    io::write_instance(std::cout, generate_instance(rng, config));
    std::cout << std::endl;
}
