#ifndef STRIP_PACKING_IO_HPP
#define STRIP_PACKING_IO_HPP

#include "blend2d/api.h"
#include "blend2d/context.h"
#include "blend2d/fontface.h"
#include "blend2d/format.h"
#include "blend2d/geometry.h"
#include "blend2d/image.h"
#include "blend2d/rgba.h"
#include "defs.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include <numeric>
#include <ostream>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <blend2d.h>

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

static instance_t read_instance(std::istream& input) {
    return YAML::Load(input).as<instance_t>();
}

static std::ostream& write_instance(std::ostream& output,
                                    const instance_t& instance) {
    return output << YAML::Node(instance);
}

static void print_instance(instance_t instance) {
    std::cout << "Recipient length: " << instance.recipient_length << std::endl;
    std::cout << "Rects (" << instance.rects.size() << "): " << std::endl;
    for (const auto& rect : instance.rects) {
        std::cout << rect.length << "x" << rect.height << "(" << rect.weight
                  << ") ";
    }
    std::cout << std::endl << std::endl;
}

static void print_solution(instance_t instance, solution_t solution) {
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

static void render_solution(const instance_t& instance, solution_t solution, std::string filename) {
    normalize(instance, solution);

    dim_type width = instance.recipient_length;
    double scale = 16;
    size_t padding = 32;

    dim_type height = 0;
    for (auto& level : solution) {
        dim_type level_height = 0;
        for (auto& i : level) {
            level_height = std::max(level_height, instance.rects[i].height);
        }
        height += level_height;
    }

    BLImage img(std::ceil(width * scale + 2 * padding),
                std::ceil(height * scale + 2 * padding), BL_FORMAT_PRGB32);

    BLContext ctx(img);
    BLRgba32 white(0xFFFFFFFF);
    ctx.fillAll(white);

    ctx.setStrokeWidth(4);

    BLRgba32 black(0xFF000000);
    ctx.strokeRect(BLRect(padding, padding, instance.recipient_length * scale,
                          height * scale),
                   black);

    double x;
    double y = height * scale + padding;

    for (auto& level : solution) {
        ctx.setStrokeWidth(1);
        x = padding;
        dim_type level_height = 0;
        for (auto& i : level) {
            auto rect = instance.rects[i];

            BLRect bl_rect(x, y - rect.height * scale, rect.length * scale,
                           rect.height * scale);
            ctx.fillRect(bl_rect, BLRgba32(0xFF, 0x00, 0x00,
                                           std::ceil(0xFF * rect.weight / 10)));
            ctx.strokeRect(bl_rect, black);

            x += rect.length * scale;
            level_height = std::max(level_height, rect.height);
        }
        y -= level_height * scale;
        ctx.setStrokeWidth(2);
        ctx.strokeLine(BLLine(padding, y, padding + width * scale, y), black);
    }

    ctx.end();

    img.writeToFile(filename.c_str());
}

}; // namespace strip_packing::io

#endif // STRIP_PACKING_IO_HPP
