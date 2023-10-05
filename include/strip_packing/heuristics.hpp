#ifndef STRIP_PACKING_HEURISTICS_HPP
#define STRIP_PACKING_HEURISTICS_HPP

#include "brkga_mp_ipr/chromosome.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "defs.hpp"

#include "util/first_fit.hpp"
#include "util/sort.hpp"

#include <brkga_mp_ipr/brkga_mp_ipr.hpp>

#include <algorithm>
#include <cstddef>
#include <random>
#include <set>
#include <vector>

namespace strip_packing::heuristics {

namespace constructive {

static solution_t next_fit(instance_t instance,
                           std::vector<size_t> permutation) {
    solution_t solution(1);
    dim_type used = 0.0;
    for (size_t i = 0; i < permutation.size(); i++) {
        size_t j = permutation[i];
        dim_type len = instance.rects[j].length;
        used += len;
        if (used <= instance.recipient_length) {
            solution.back().push_back(j);
        } else {
            used = len;
            solution.push_back({j});
        }
    }
    return solution;
}

static solution_t first_fit(instance_t instance,
                            std::vector<size_t> permutation) {
    solution_t solution(1);

    // Construímos a solução baseado na estratégia de first-fit, adicionando
    // cada item em sequência descrescente de peso ao nível mais baixo no qual
    // ele cabe, ou criando um novo nível para ele, caso não caiba em nenhum.
    util::first_fit_tree<dim_type> levels(1, instance.recipient_length);
    for (size_t i = 0; i < permutation.size(); i++) {
        size_t j = permutation[i];
        dim_type len = instance.rects[j].length;
        size_t level = levels.first_fit(len);
        if (level != decltype(levels)::npos) {
            levels.decrease(level, len);
            solution[level].push_back(j);
        } else {
            levels.push_back(instance.recipient_length - len);
            solution.push_back({j});
        }
    }

    return solution;
}

static solution_t best_fit(instance_t instance,
                           std::vector<size_t> permutation) {
    // Construímos a solução baseado na estratégia de best-fit, adicionando
    // cada item em sequência descrescente de altura ao nível no qual ele tem o
    // "melhor encaixe", isto é, aquele em que o espaço restante ao adicionar o
    // item é mínimo.
    solution_t solution(1);

    // Usamos um conjunto ordenado para obter a menor cota superior de
    // capacidade para um ítem. Isso corresponde ao nível cuja utilização é
    // máxima dentre os níveis em que o item cabe.
    // Por questões de implementação específicas do C++, definimos algumas
    // estruturas auxiliares para o algoritmo.
    struct bin_record {
        size_t index;
        dim_type capacity;
    };
    struct compare_bin_record {
        bool operator()(const bin_record& a, const bin_record& b) const {
            return a.capacity < b.capacity;
        }
    };
    std::set<bin_record, compare_bin_record> levels;

    // Começamos com um nível com capacidade plena.
    levels.insert({0, instance.recipient_length});

    for (size_t i = 0; i < permutation.size(); i++) {
        size_t j = permutation[i];
        dim_type len = instance.rects[j].length;
        auto upper_bound = levels.upper_bound({0, len});
        if (upper_bound != levels.end()) {
            bin_record record = *upper_bound;
            record.capacity -= len;
            solution[record.index].push_back(j);

            // Removemos e adicionamos o registro do nível novamente, como
            // forma de atualizar o valor da chave.
            levels.erase(upper_bound);
            levels.insert(record);
        } else {
            levels.insert({solution.size(), instance.recipient_length - len});
            solution.push_back({j});
        }
    }

    return solution;
}

/**
 * Heurística construtiva randomizada de first-fit em ordem decrescente de
 * prioridade.
 *
 * A ideia da heurística é ordenar os itens da instância em ordem decrescente
 * de peso (com algum ruído, para permitir aleatorização) e sequencialmente
 * encaixar cada item no nível mais baixo que tem espaço suficiente para ele.
 *
 * Complexidade: O(n lg n).
 */
template <typename URBG,
          typename NoiseDist = std::uniform_real_distribution<dim_type>>
solution_t randomized_first_fit_decreasing_weight(
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
    std::vector<size_t> permutation = util::sort_permutation(
        instance.rects,
        [](const auto& a, const auto& b) { return a.weight > b.weight; });

    return first_fit(instance, permutation);
}

/**
 * Heurística construtiva randomizada de best-fit em ordem crescente de altura.
 *
 * A ideia da heurística é tentar minimizar a altura da pilha, priorizando
 * retângulos mais baixos no começo (com a intuição de que níveis iniciais
 * altos têm maior impacto que níveis finais altos).
 *
 * Complexidade: O(n lg n).
 */
template <typename URBG,
          typename NoiseDist = std::uniform_real_distribution<dim_type>>
solution_t randomized_best_fit_increasing_height(
    instance_t instance, URBG&& rng,
    NoiseDist noise = std::uniform_real_distribution<>(-1.0, 1.0)) {

    // Aplica ruído aos retângulos na instância.
    // Isso não modifica a instância original, visto que ela é passada por cópia
    // para a função da heurística.
    for (auto& rect : instance.rects) {
        rect.height = std::max(0.0, rect.height + noise(rng));
    }

    // Computamos um vetor de permutação para a ordenação por peso.
    // Isso é feito (no lugar de ordenar a lista de retângulos por peso
    // diretamente, por exemplo) para permitir referenciar a posição original de
    // cada retângulo na instância original.
    std::vector<size_t> permutation = util::sort_permutation(
        instance.rects,
        [](const auto& a, const auto& b) { return a.height < b.height; });

    return best_fit(instance, permutation);
}

} // namespace constructive

namespace improvement {

template <typename URBG>
solution_t brkga_mp_ipr_improve(instance_t instance,
                                std::vector<solution_t> initial, URBG&& rng,
                                BRKGA::BrkgaParams brkga_params,
                                BRKGA::ControlParams control_params,
                                const unsigned max_threads = 1) {
    struct NextFitDecoder {
        instance_t m_instance;

        NextFitDecoder(instance_t instance) : m_instance(instance) {}

        solution_t rebuild(BRKGA::Chromosome chromosome) {
            std::vector<size_t> permutation =
                util::sort_permutation(chromosome);
            return constructive::next_fit(m_instance, permutation);
        }

        BRKGA::fitness_t decode(BRKGA::Chromosome chromosome, bool rewrite) {
            return m_instance.cost(rebuild(chromosome));
        }
    } decoder(instance);

    const size_t chromosome_size = instance.rects.size();

    BRKGA::BRKGA_MP_IPR<NextFitDecoder> algorithm(
        decoder, BRKGA::Sense::MINIMIZE, rng(), chromosome_size, brkga_params,
        max_threads);

    std::vector<BRKGA::Chromosome> initial_chromosomes;
    initial_chromosomes.reserve(initial.size());
    for (const auto& solution : initial) {
        BRKGA::Chromosome chromosome;
        chromosome.resize(chromosome_size);
        size_t i = 0;
        for (const auto& level : solution) {
            for (const auto& index : level) {
                chromosome[index] = i++ / double(chromosome_size);
            }
        }
        initial_chromosomes.push_back(chromosome);
    }

    algorithm.setInitialPopulation(initial_chromosomes);

    auto status = algorithm.run(control_params);
    std::cout << "Ran " << status.current_iteration << " iterations"
              << std::endl;

    return decoder.rebuild(status.best_chromosome);
}

} // namespace improvement

} // namespace strip_packing::heuristics

#endif // STRIP_PACKING_HEURISTICS_HPP
