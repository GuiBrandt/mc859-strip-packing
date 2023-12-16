#ifndef STRIP_PACKING_IO_HPP
#define STRIP_PACKING_IO_HPP

#include "defs.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include <numeric>
#include <ostream>

#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<strip_packing::rect_t> {
    static Node encode(const strip_packing::rect_t& rect) {
        Node node;
        node["height"] = rect.height;
        node["length"] = rect.length;
        node["weight"] = rect.weight;
        return node;
    }

    static bool decode(const Node& node, strip_packing::rect_t& rect) {
        rect.height = node["height"].as<strip_packing::dim_type>();
        rect.length = node["length"].as<strip_packing::dim_type>();
        rect.weight = node["weight"].as<strip_packing::cost_type>();
        return true;
    }
};

template <> struct convert<strip_packing::instance_t> {
    static Node encode(const strip_packing::instance_t& instance) {
        Node node;
        node["recipient_length"] = instance.recipient_length;
        for (const auto& rect : instance.rects) {
            node["rects"].push_back(rect);
        }
        return node;
    }

    static bool decode(const Node& node, strip_packing::instance_t& instance) {
        instance.recipient_length =
            node["recipient_length"].as<strip_packing::dim_type>();

        for (const auto& rect : node["rects"]) {
            instance.rects.push_back(rect.as<strip_packing::rect_t>());
        }

        return true;
    }
};
} // namespace YAML

namespace strip_packing::io {

static inline instance_t read_instance(std::istream& input) {
    return YAML::Load(input).as<instance_t>();
}

static inline std::ostream& write_instance(std::ostream& output,
                                           const instance_t& instance) {
    return output << YAML::Node(instance);
}

static inline std::ostream& print_instance(std::ostream& out,
                                           instance_t instance) {
    out << "Recipient length: " << instance.recipient_length << std::endl;
    out << "Rects (" << instance.rects.size() << "): " << std::endl;
    for (const auto& rect : instance.rects) {
        out << rect.length << "x" << rect.height << "(" << rect.weight << ") ";
    }
    out << std::endl << std::endl;
    return out;
}

static inline std::ostream& print_solution(std::ostream& out,
                                           const instance_t& instance,
                                           const solution_t& solution) {
    out << "Cost: " << instance.cost(solution) << std::endl;
    dim_type h = 0;
    for (size_t i = 0; i < solution.size(); i++) {
        out << "(level " << std::right << std::setw(2) << i
            << ", h = " << std::setw(4) << h << ") ";
        dim_type max_h = 0;
        dim_type L = 0;
        for (const auto& j : solution[i]) {
            L += instance.rects[j].length;
            out << std::setw(3) << std::right << j << ":" << std::setw(3)
                << std::left << instance.rects[j].weight << " ";
            max_h = std::max(max_h, instance.rects[j].height);
        }
        out << " (L = " << L << ")";
        h += max_h;
        out << std::endl;
    }
    return out;
}

}; // namespace strip_packing::io

#endif // STRIP_PACKING_IO_HPP
