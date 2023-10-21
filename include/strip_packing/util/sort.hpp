#ifndef STRIP_PACKING_UTIL_SORT_HPP
#define STRIP_PACKING_UTIL_SORT_HPP

#include <algorithm>
#include <vector>

namespace strip_packing::util {

template <typename T, typename Compare = std::less<T>>
std::vector<size_t> sort_permutation(std::vector<T> vec, Compare compare = std::less<T>()) {
    std::vector<size_t> permutation;
    permutation.resize(vec.size());
    for (size_t i = 1; i < vec.size(); i++) {
        permutation[i] = i;
    }
    std::sort(permutation.begin(), permutation.end(), [&](size_t i, size_t j) {
        return compare(vec[i], vec[j]);
    });
    return permutation;
}

}; // namespace strip_packing::util

#endif // STRIP_PACKING_UTIL_SORT_HPP
