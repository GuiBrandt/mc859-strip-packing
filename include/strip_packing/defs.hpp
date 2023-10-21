#ifndef STRIP_PACKING_DEFS_HPP
#define STRIP_PACKING_DEFS_HPP

#include <algorithm>
#include <limits>
#include <numeric>
#include <vector>

namespace strip_packing {

using dim_type = double;
using cost_type = double;

/*! Estrutura para um retângulo no problema. */
struct rect_t {
    dim_type length;  /// Largura
    dim_type height;  /// Altura
    cost_type weight; /// Peso/prioridade
};

/*! Estrutura para uma instância do problema. */
struct instance_t {
    std::vector<rect_t> rects; /// Conjunto de retângulos
    dim_type recipient_length; /// Largura do recipiente

    /*! Tipo para um subconjunto de retângulos. */
    using rect_subset = std::vector<size_t>;

    /*! Tipo para uma partição ordenada do conjunto de retângulos. */
    using partition_t = std::vector<rect_subset>;

    /**
     * Determina se uma partição do conjunto de retângulos é uma solução viável
     * para o problema.
     */
    bool viable(const partition_t& partition) const {
        const dim_type L = recipient_length;

        // Uma solução é viável se a soma das larguras dos retângulos em cada
        // nível é menor ou igual à largura do recipiente.
        for (const auto& part : partition) {
            dim_type total_length = dim_type(0);
            for (size_t i : part) {
                total_length += rects[i].length;
            }
            if (total_length > L) {
                return false;
            }
        }
        return true;
    }

    /*! Computa o custo de uma solução para o problema. */
    cost_type cost(const partition_t& solution) const {
        cost_type total = 0;
        dim_type height = 0;

        for (size_t level = 0; level < solution.size(); level++) {
            dim_type level_height = 0;

            for (const auto& index : solution[level]) {
                // A altura de um nível é a maior altura de um retângulo
                // naquele nível.
                level_height = std::max(level_height, rects[index].height);

                // O custo de uma solução é dado pela soma dos produtos dos
                // pesos de cada retângulo com sua altura na solução.
                total += rects[index].weight * height;
            }

            // A altura de cada item no próximo nível corresponde à altura da
            // base do nível atual, mais a altura desse nível.
            height += level_height;
        }

        return total;
    }
};

/*! Tipo para uma solução do problema. */
using solution_t = instance_t::partition_t;

static void normalize(const instance_t& instance, solution_t& solution) {
    for (auto& level : solution) {
        std::sort(level.begin(), level.end(), [&instance](size_t i, size_t j) {
            return instance.rects[i].height > instance.rects[j].height;
        });
    }
}

} // namespace strip_packing

#endif // STRIP_PACKING_DEFS_HPP
