#ifndef STRIP_PACKING_IO_HPP
#define STRIP_PACKING_IO_HPP

#include "defs.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include <numeric>
#include <ostream>

#include <mylib/myutils.h>

namespace strip_packing::io {

static inline instance_t read_instance(std::istream& input) {
    using namespace mylib;

    instance_t instance;

    int N;
    StringTable TabHeader(1, input);
    if (!TabHeader.first("nitems", N)) {
        cout << "Erro: Leitura do campo \"nitems\" no arquivo" << endl;
    }

    int L;
    if (!TabHeader.first("strip_width", L)) {
        cout << "Erro: Leitura do campo \"strip_width\" no arquivo " << endl;
    }

    if (L < 0) {
        cout << "Erro: Largura da faixa não pode ser negativa." << endl;
        exit(1);
    }
    instance.recipient_length = L;
    instance.rects.resize(N);
    StringTable TabItems(N, input);
    std::vector<int> v_item_id(N);
    std::vector<int> v_item_width(N);
    std::vector<int> v_item_height(N);
    std::vector<int> v_item_weight(N);
    TabItems.readcolumn("item", v_item_id);
    TabItems.readcolumn("width", v_item_width);
    TabItems.readcolumn("height", v_item_height);
    TabItems.readcolumn("weight", v_item_weight);
    for (int i = 0; i < N; i++) {
        int item_id = v_item_id[i];
        if ((item_id < 0) || (item_id >= N)) {
            std::cerr << "Erro: Numero do item fora do intervalo 0 a " << N - 1
                      << std::endl;
            exit(1);
        }
        if (instance.rects[item_id].length != 0) {
            std::cerr << "Erro: Item " << item_id << " duplicado." << std::endl;
            exit(1);
        }
        if ((v_item_width[i] < 0) || (v_item_width[i] > L) ||
            v_item_height[i] <= 0) {
            std::cerr << "Erro: Dimensoes do item " << item_id
                      << " invalidas: (" << v_item_width[i] << ","
                      << v_item_height[i] << ")" << std::endl;
            exit(1);
        }
        instance.rects[item_id].length = v_item_width[i];
        instance.rects[item_id].height = v_item_height[i];
        instance.rects[item_id].weight = v_item_weight[i];
    }
    return instance;
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
