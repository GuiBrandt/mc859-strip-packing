#ifndef STRIP_PACKING_UTIL_FIRST_FIT_TREE_HPP
#define STRIP_PACKING_UTIL_FIRST_FIT_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace strip_packing::util {

/**
 * Classe de árvore para first-fit.
 *
 * A árvore corresponde a uma árvore binária completa, e segue uma estrutura
 * implícita em um vetor (números correspondem a índices no vetor, começando em
 * 1):
 *
 *                8
 *        4                 12
 *    2       6       10          14
 *  1   3   5   7   9    11    13    15
 *
 * @param T - tipo de valor dos elementos da árvore.
 * @param Compare - comparador de elementos.
 * @param Allocator - alocador de memória.
 */
template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>>
class first_fit_tree {
  private:
    using node_t = size_t;

    Compare m_compare;
    Allocator m_alloc;

    size_t m_size; /// Tamanho (número de elementos) da árvore

    std::vector<T, Allocator> m_data;    /// Vetor de dados da árvore
    std::vector<T, Allocator> m_summary; /// Vetor de dados agregados da árvore

    /**
     * Determina a altura de um nó na árvore. O(1).
     *
     * A altura é definida pelo tamanho do menor caminho entre o nó e uma folha.
     */
    static constexpr inline size_t height(node_t node) {
        return ffsl(node + 1) - 1;
    }

    /*! Determina se um nó é uma folha. O(1). */
    static constexpr bool leaf(node_t node) { return height(node) == 0; }

    /*! Nó "pai" de um nó da árvore. O(1). */
    static constexpr inline node_t parent(node_t node) {
        size_t h = height(node);
        bool isleft = ((node + 1) & (1 << (h + 1))) == 0;
        return node + ((isleft - !isleft) << h);
    }

    /*! Filho esquerdo de um nó da árvore. O(1). */
    static constexpr inline node_t left_child(node_t node) {
        size_t h = height(node);
        return node - ((h > 0) << (h - 1));
    }

    /*! Filho direito de um nó da árvore. O(1). */
    static constexpr inline node_t right_child(node_t node) {
        size_t h = height(node);
        return node + ((h > 0) << (h - 1));
    }

    /*! Raíz da árvore. O(1). */
    constexpr inline node_t root() const { return m_data.size() / 2; }

    /**
     * Redimensiona os vetores para a próxima potência de dois capaz de acomodar
     * a capacidade dada. O(n).
     */
    void resize_vectors(size_t new_cap) {
        assert(new_cap > m_data.size());
        size_t cap = (1 << (32 - __builtin_clz(new_cap))) - 1;
        m_data.resize(cap);
        m_summary.resize(cap);
    }

    /*! Recomputa o máximo para uma subárvore. O(1). */
    inline void refresh_max(node_t node) {
        node_t l = left_child(node), r = right_child(node);
        m_summary[node] = std::max(m_summary[l], m_summary[r]);
        if (m_compare(m_summary[node], m_data[node])) {
            m_summary[node] = m_data[node];
        }
    }

    /*! Recomputa o máximo para a árvore toda. O(n). */
    void bottom_up_refresh() {
        for (size_t stride = 2; stride <= m_summary.size() + 1; stride *= 2) {
            for (size_t i = (stride >> 1) - 1; i < m_size; i += stride) {
                refresh_max(i);
            }
        }
    }

  public:
    using value_type = T;
    using compare = Compare;
    using allocator_type = Allocator;

    using reference = value_type&;
    using const_reference = const value_type&;

    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer =
        typename std::allocator_traits<Allocator>::const_pointer;

    using iterator = typename decltype(m_data)::iterator;
    using const_iterator = typename decltype(m_data)::const_iterator;

    using reverse_iterator = typename decltype(m_data)::reverse_iterator;
    using const_reverse_iterator =
        typename decltype(m_data)::const_reverse_iterator;

    using difference_type = ptrdiff_t;

    using size_type = size_t;

    static constexpr size_type npos = std::numeric_limits<size_type>::max();

    /*! Constrói uma árvore vazia. */
    first_fit_tree(const Compare& compare = Compare(),
                   const Allocator& alloc = Allocator())
        : m_compare(compare), m_alloc(alloc), m_size(0), m_data(m_alloc),
          m_summary(m_alloc) {}

    /*! Constrói uma árvore com tamanho e valores fixos. */
    first_fit_tree(size_type size, value_type value,
                   const Compare& compare = Compare(),
                   const Allocator& alloc = Allocator())
        : m_compare(compare), m_alloc(alloc), m_size(size), m_data(m_alloc),
          m_summary(m_alloc) {
        resize_vectors(m_size);
        std::fill(m_data.begin(), m_data.begin() + m_size, value);
        bottom_up_refresh();
    }

    /*! Constrói uma árvore a partir de uma sequência. */
    template <typename InputIterator>
    first_fit_tree(InputIterator first, InputIterator last,
                   const Compare& compare = Compare(),
                   const Allocator& alloc = Allocator())
        : m_compare(compare), m_alloc(alloc),
          m_size(std::distance(first, last)), m_data(m_alloc),
          m_summary(m_alloc) {
        resize_vectors(m_size);
        std::copy(first, last, m_data.begin());
        bottom_up_refresh();
    }

    iterator begin() { return m_data.begin(); }
    iterator end() { return m_data.begin() + m_size; }

    const_iterator begin() const { return m_data.begin(); }
    const_iterator end() const { return m_data.begin() + m_size; }

    const_iterator cbegin() const { return m_data.cbegin(); }
    const_iterator cend() const { return m_data.cbegin() + m_size; }

    reverse_iterator rbegin() const {
        return m_data.rbegin() + (m_data.size() - m_size);
    }

    reverse_iterator rend() const { return m_data.rend(); }

    bool empty() const { return m_data.empty(); }

    /*! Tamanho (número de elementos) no conjunto. */
    size_type size() const { return m_size; }

    size_type max_size() const { return m_data.max_size(); }

    size_type capacity() const { return m_data.capacity(); }

    /**
     * Reserva espaço suficiente na estrutura para acomodar um número dado de
     * elementos. O(n).
     */
    void reserve(size_type new_cap) {
        if (empty()) {
            m_summary.resize(1);
            m_data.resize(1);
        } else if (new_cap > m_data.size()) {
            T value = m_summary[root()];
            node_t node = root(), next = node;
            resize_vectors(new_cap);
            for (node = next; node < m_summary.size(); node = parent(node)) {
                if (m_compare(m_summary[node], value)) {
                    m_summary[node] = value;
                } else {
                    break;
                }
            }
        }
    }

    /*! Encontra o primeiro índice com valor maior ou igual ao valor dado. */
    size_t first_fit(value_type value) const {
        node_t node = root();
        if (empty() || m_compare(m_summary[node], value)) {
            return npos;
        }

        while (!leaf(node)) {
            node_t l = left_child(node);
            if (!m_compare(m_summary[l], value)) {
                node = l;
            } else if (!m_compare(m_data[node], value)) {
                break;
            } else {
                node_t r = right_child(node);
                assert(!m_compare(m_summary[r], value));
                node = r;
            }
        }

        return node;
    }

    /*! Diminui o valor de uma posição no conjunto. */
    void decrease(size_type index, T delta) {
        node_t node = index;
        m_data[node] -= delta;

        if (leaf(node)) {
            m_summary[node] = m_data[node];
            node = parent(node);
        }

        while (node < m_summary.size()) {
            refresh_max(node);
            node = parent(node);
        }
    }

    /*! Adiciona um valor ao fim do conjunto. */
    void push_back(T value) {
        reserve(m_size + 1);

        m_data[m_size] = value;
        if (m_compare(m_summary[m_size], value)) {
            m_summary[m_size] = value;
        }

        for (node_t node = parent(m_size); node < m_summary.size();
             node = parent(node)) {
            if (m_compare(m_summary[node], value)) {
                m_summary[node] = value;
            } else {
                break;
            }
        }

        m_size++;
    }

    reference operator[](size_type index) { return m_data[index]; }
    const_reference operator[](size_type index) const { return m_data[index]; }
};

} // namespace strip_packing::util

#endif // STRIP_PACKING_UTIL_FIRST_FIT_TREE_HPP
