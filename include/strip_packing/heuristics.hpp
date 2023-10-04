#ifndef STRIP_PACKING_HEURISTICS_HPP
#define STRIP_PACKING_HEURISTICS_HPP

#include "defs.hpp"
#include "util/first_fit_tree.hpp"

#include <random>
#include <vector>

namespace strip_packing::heuristics {

namespace constructive {

/**
 * Heurística construtiva randomizada de first-fit em ordem decrescente.
 *
 * A ideia da heurística é ordenar os itens da instância em ordem decrescente
 * de peso (com algum ruído, para permitir aleatorização) e sequencialmente
 * encaixar cada ítem no nível mais baixo que tem espaço suficiente para ele.
 *
 * Complexidade: O(n lg n).
 */
template <typename URBG,
          typename NoiseDist = std::uniform_real_distribution<cost_type>>
static solution_t first_fit_decreasing_random(
    instance_t instance, URBG&& rng,
    NoiseDist noise = std::uniform_real_distribution<>(-1.0, 1.0)) {

    // Aplica ruído aos retângulos na instância.
    // Isso não modifica a instância original, visto que ela é passada por cópia
    // para a função da heurística.
    for (auto& rect : instance.rects) {
        rect.weight = std::max(0.0, rect.weight + noise(rng));
    }

    // Computamos um vetor de permutação para a ordenação por peso.
    // Isso é feito (no lugar de ordenar a lista de retângulos por peso
    // diretamente, por exemplo) para permitir referenciar a posição original de
    // cada retângulo na instância original.
    std::vector<size_t> permutation;
    permutation.reserve(instance.rects.size());
    for (int i = 0; i < instance.rects.size(); i++) {
        permutation.push_back(i);
    }

    std::sort(permutation.begin(), permutation.end(),
              [&instance](size_t i, size_t j) {
                  return instance.rects[i].weight > instance.rects[j].weight;
              });

    // Construímos a solução baseado na estratégia de first-fit, adicionando
    // cada ítem em sequência descrescente de peso a nível mais baixo no qual
    // ele cabe, ou criando um novo nível para ele, caso não caiba em nenhum.
    solution_t solution(1);
    util::first_fit_tree<dim_type> fft(1, instance.recipient_length);
    for (size_t i = 0; i < permutation.size(); i++) {
        size_t j = permutation[i];
        dim_type len = instance.rects[j].length;
        size_t level = fft.first_fit(len);
        if (level != decltype(fft)::npos) {
            fft.decrease(level, len);
            solution[level].push_back(j);
        } else {
            fft.push_back(instance.recipient_length - len);
            solution.push_back({j});
        }
    }
    return solution;
}

} // namespace constructive

namespace improvement {}

} // namespace strip_packing::heuristics

#endif // STRIP_PACKING_HEURISTICS_HPP
