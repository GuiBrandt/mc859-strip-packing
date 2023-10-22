#include <strip_packing.hpp>
#include <strip_packing/io.hpp>

using namespace strip_packing;

template <typename URBG> instance_t generate_instance(URBG&& rng) {
    std::uniform_int_distribution<size_t> size_dist(300, 500);
    std::uniform_real_distribution<dim_type> cap_dist(75, 100);
    std::uniform_real_distribution<dim_type> dim_dist(3, 10);
    std::uniform_real_distribution<cost_type> weight_dist(0, 10);

    size_t instance_size = size_dist(rng);
    instance_t instance;
    instance.recipient_length = cap_dist(rng);
    instance.rects.reserve(instance_size);
    for (size_t i = 0; i < instance_size; i++) {
        instance.rects.push_back(
            {dim_dist(rng), dim_dist(rng), weight_dist(rng)});
    }

    return instance;
}

int main() {
    
}
