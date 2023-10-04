#ifndef STRIP_PACKING_UTIL_SEGTREE_HPP
#define STRIP_PACKING_UTIL_SEGTREE_HPP

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace strip_packing::util {

/**
 * Classe de árvore para first-fit.
 *
 * @param T - tipo de valor dos elementos da árvore.
 * @param Allocator - alocador de memória.
 */
template <typename T, typename Allocator = std::allocator<T>>
class first_fit_tree {
  private:
    using node_t = size_t;

    Allocator m_alloc;
    size_t m_size; /// Tamanho (número de elementos) da árvore

    std::vector<T, Allocator> m_data; /// Vetor de dados da árvore

    /**
     * Altura de um nó na árvore.
     *
     * Determinado pelo tamanho do menor caminho entre o nó e uma folha.
     */
    static constexpr inline size_t height(node_t node) {
        return __builtin_ffsl(node + 1);
    }

    /*! Determina se um nó é uma folha. */
    static constexpr bool leaf(node_t node) { return height(node) == 1; }

    /*! Nó "pai" de um nó da árvore. */
    static constexpr inline node_t parent(node_t node) {
        size_t h = height(node);
        bool isleft = (node & (1 << h)) == 0;
        return node + (1 << (h - 1)) * (isleft - !isleft);
    }

    /*! Filho esquerdo de um nó da árvore. */
    static constexpr inline node_t left_child(node_t node) {
        size_t h = height(node);
        return node - (h > 1) * (1 << (h - 2));
    }

    /*! Filho direito de um nó da árvore. */
    static constexpr inline node_t right_child(node_t node) {
        size_t h = height(node);
        return node + (h > 1) * (1 << (h - 2));
    }

    constexpr inline node_t root() const { return m_data.size() / 2; }

    void resize_vectors(size_t new_cap) {
        size_t cap = (1 << (32 - __builtin_clz(new_cap))) - 1;
        m_data.resize(cap);
        m_summary.resize(cap);
    }

    void refresh_max(node_t node) {
        node_t l = left_child(node), r = right_child(node);
        m_summary[node] = std::max(m_summary[l], m_summary[r]);
        if (m_data[node] > m_summary[node]) {
            m_summary[node] = m_data[node];
        }
    }

    void bottom_up_refresh() {
        for (int stride = 2; stride <= m_summary.size() + 1; stride *= 2) {
            for (int i = (stride >> 1) - 1; i < m_size; i += stride) {
                refresh_max(i);
            }
        }
    }

  public:
    std::vector<T, Allocator> m_summary; /// Vetor de dados agregados da árvore

    using value_type = T;
    using allocator_type = Allocator;

    using reference = typename allocator_type::reference;
    using const_reference = typename allocator_type::const_reference;

    using pointer = typename allocator_type::pointer;
    using const_pointer = typename allocator_type::const_pointer;

    using iterator = typename decltype(m_data)::iterator;
    using const_iterator = typename decltype(m_data)::const_iterator;

    using reverse_iterator = typename decltype(m_data)::reverse_iterator;
    using const_reverse_iterator =
        typename decltype(m_data)::const_reverse_iterator;

    using difference_type = ptrdiff_t;

    using size_type = size_t;

    static constexpr size_type npos = std::numeric_limits<size_type>::max();

    first_fit_tree(const Allocator& alloc = Allocator())
        : m_alloc(alloc), m_size(0), m_data(m_alloc), m_summary(m_alloc) {}

    first_fit_tree(size_type size, value_type value,
                   const Allocator& alloc = Allocator())
        : m_alloc(alloc), m_size(size), m_data(m_alloc), m_summary(m_alloc) {
        resize_vectors(m_size);
        std::fill(m_data.begin(), m_data.begin() + m_size, value);
        std::fill(m_summary.begin(), m_summary.begin() + m_size, value);
        bottom_up_refresh();
    }

    template <typename InputIterator>
    first_fit_tree(InputIterator first, InputIterator last,
                   const Allocator& alloc = Allocator())
        : m_alloc(alloc), m_size(std::distance(first, last)), m_data(m_alloc),
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

    size_type size() const { return m_size; }

    size_type max_size() const { return m_data.max_size(); }

    size_type capacity() const { return m_data.capacity(); }

    void reserve(size_type new_cap) {
        if (empty()) {
            m_summary.resize(1);
            m_data.resize(1);
        } else if (new_cap > m_data.size()) {
            T value = m_summary[root()];
            node_t node = root(), next = node;
            resize_vectors(new_cap);
            for (node = next; node < m_summary.size(); node = parent(node)) {
                if (value > m_summary[node]) {
                    m_summary[node] = value;
                } else {
                    break;
                }
            }
        }
    }

    size_type first_fit(value_type value) const {
        node_t node = root();
        if (empty() || m_summary[node] < value) {
            return npos;
        }

        while (!leaf(node)) {
            node_t l = left_child(node);
            if (m_summary[l] >= value) {
                node = l;
            } else if (m_data[node] >= value) {
                break;
            } else {
                node_t r = right_child(node);
                assert(m_summary[r] >= value);
                node = r;
            }
        }

        return node;
    }

    void decrease(size_type index, T delta) {
        node_t node = index;

        size_t value = m_data[node];
        m_data[node] = value - delta;

        if (leaf(node)) {
            m_summary[node] = m_data[node];
            node = parent(node);
        }

        while (node < m_summary.size()) {
            refresh_max(node);
            node = parent(node);
        }
    }

    void push_back(T value) {
        reserve(m_size + 1);

        m_data[m_size] = value;
        if (value > m_summary[m_size]) {
            m_summary[m_size] = value;
        }

        for (node_t node = parent(m_size); node < m_summary.size();
             node = parent(node)) {
            if (value > m_summary[node]) {
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

#endif // STRIP_PACKING_UTIL_SEGTREE_HPP
