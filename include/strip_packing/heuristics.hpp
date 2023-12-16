#ifndef STRIP_PACKING_HEURISTICS_HPP
#define STRIP_PACKING_HEURISTICS_HPP

#include "defs.hpp"

#include "io.hpp"
#include "render.hpp"

#include "util/first_fit.hpp"
#include "util/sort.hpp"

#include <brkga_mp_ipr/brkga_mp_ipr.hpp>

#include <algorithm>
#include <cstddef>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace strip_packing::heuristics {

namespace constructive {

/**
 * Heurística construtiva determinística de next-fit. O(n).
 *
 * Insere retângulos em ordem, criando um novo nível sempre que um retângulo
 * não couber no nível atual.
 */
static inline solution_t next_fit(const instance_t& instance,
                                  const std::vector<size_t>& permutation) {
    solution_t solution;
    dim_type used = instance.recipient_length;
    for (size_t i = 0; i < permutation.size(); i++) {
        size_t j = permutation[i];
        auto rect = instance.rects[j];
        dim_type len = rect.length;
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

/**
 * Heurística construtiva determinística de first-fit. O(n lg n).
 */
static inline solution_t first_fit(const instance_t& instance,
                                   const std::vector<size_t>& permutation) {
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

/**
 * Heurística construtiva determinística de best-fit. O(n lg n).
 */
static inline solution_t best_fit(const instance_t& instance,
                                  const std::vector<size_t>& permutation) {
    // Construímos a solução baseado na estratégia de best-fit, adicionando
    // cada item em sequência descrescente de altura ao nível no qual ele tem o
    // "melhor encaixe", isto é, aquele em que o espaço restante ao adicionar o
    // item é mínimo.
    solution_t solution(1);

    // Usamos um conjunto ordenado para obter a menor cota superior de
    // capacidade para um ítem. Isso corresponde ao nível cuja capacidade é
    // mínima dentre os níveis em que o item cabe.
    // Por questões de implementação específicas do C++, definimos algumas
    // estruturas auxiliares para o algoritmo.
    struct bin_record {
        size_t index;
        dim_type capacity;
    };
    struct compare_bin_record {
        bool operator()(const bin_record& a, const bin_record& b) const {
            return a.capacity <= b.capacity;
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
 * Heurística construtiva randomizada de first-fit em ordem decrescente da
 * proporção entre prioridade e altura. O(n lg n).
 *
 * A ideia da heurística é tentar ordenar os itens da instância em ordem
 * decrescente de densidade (com algum ruído, para permitir aleatorização), e
 * sequencialmente encaixar cada item no nível mais baixo que tem espaço
 * suficiente para ele.
 */
template <typename URBG,
          typename NoiseDist = std::uniform_real_distribution<cost_type>>
solution_t randomized_first_fit_decreasing_density(
    instance_t instance, URBG&& rng,
    NoiseDist noise = std::uniform_real_distribution<>(-1.0, 1.0)) {

    // Aplica ruído aos retângulos na instância.
    // Isso não modifica a instância original, visto que ela é passada por cópia
    // para a função da heurística.
    for (auto& rect : instance.rects) {
        rect.weight = std::max(0.0, rect.weight + noise(rng));
    }

    // Computamos um vetor de permutação para a ordenação por densidade.
    // Isso é feito (no lugar de ordenar a lista de retângulos diretamente, por
    // exemplo) para permitir referenciar a posição original de cada retângulo
    // na instância original.
    std::vector<size_t> permutation = util::sort_permutation(
        instance.rects, [](const auto& a, const auto& b) {
            return a.weight * b.area() > b.weight * a.area();
        });

    return first_fit(instance, permutation);
}

/**
 * Heurística construtiva randomizada de best-fit em ordem crescente de altura.
 * O(n lg n).
 *
 * A ideia da heurística é tentar minimizar a altura da pilha, priorizando
 * retângulos mais baixos no começo (com a intuição de que níveis iniciais
 * altos têm maior impacto que níveis finais altos).
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

/**
 * Heurística de melhoria com BRKGA-MP-IPR.
 *
 * Recebe uma instância do problema e uma lista de soluções iniciais e melhora
 * elas com um algoritmo genético de chave aleatória enviesado com múltiplos
 * pais e implicit path-relinking.
 */
class brkga_mp_ipr {
  public:
    brkga_mp_ipr(const instance_t& instance,
                 const std::vector<solution_t>& initial)
        : m_instance(instance), m_initial(initial) {}

    /*! Tamanho do cromossomo usado no algoritmo. */
    size_t chromosome_size() const { return m_instance.rects.size(); }

    /**
     * Executa o algoritmo com os parâmetros dados.
     */
    template <typename URBG>
    solution_t run(URBG&& rng, BRKGA::BrkgaParams brkga_params,
                   BRKGA::ControlParams control_params,
                   unsigned max_threads = 1) const {
        next_fit_decoder decoder(m_instance);

        brkga_params.custom_shaking = shaking_function(rng, decoder);

        algorithm brkga(decoder, BRKGA::Sense::MINIMIZE, rng(),
                        chromosome_size(), brkga_params, max_threads);

        set_initial_population(brkga);
        observe_solution_progress(brkga);

        auto status = brkga.run(control_params);
        std::cout << "Ran " << status.current_iteration << " iterations"
                  << std::endl;

        return decoder.rebuild(status.best_chromosome);
    }

  private:
    using chromosome = BRKGA::Chromosome;

    /*! Decodificador de solução a partir de um cromossomo. */
    struct next_fit_decoder {
        instance_t m_instance;

        next_fit_decoder(instance_t instance) : m_instance(instance) {}

        solution_t rebuild(chromosome chromosome) const {
            // A solução determinada por um cromossomo é uma obtida pela
            // estratégia "next fit", inserindo os retângulos por ordem
            // crescente dos valores correspondentes a cada um no cromossomo.
            std::vector<size_t> permutation =
                util::sort_permutation(chromosome);
            return constructive::next_fit(m_instance, permutation);
        }

        BRKGA::fitness_t decode(chromosome chromosome, bool) const {
            return m_instance.cost(rebuild(chromosome));
        }
    };

    /*! Codifica uma solução na forma de cromossomo. */
    chromosome encode(const solution_t& solution) const {
        size_t S = chromosome_size();
        chromosome chromosome(S);

        // Construímos o cromossomo de forma que a ordem em que os retângulos
        // serão inseridos durante a decodificação seja igual à ordem em que os
        // retângulos aparecem na solução, da esquerda para a direita e de baixo
        // (nível 0) para cima.
        size_t i = 0;
        for (const auto& level : solution) {
            for (const auto& index : level) {
                chromosome[index] = i++ / double(S);
            }
        }

        return chromosome;
    }

    using algorithm = BRKGA::BRKGA_MP_IPR<next_fit_decoder>;

    /*! Cria a população inicial do algoritmo. */
    void set_initial_population(algorithm& brkga) const {
        std::vector<chromosome> population(m_initial.size());
        std::transform(
            m_initial.begin(), m_initial.end(), population.begin(),
            [this](const solution_t& solution) { return encode(solution); });
        brkga.setInitialPopulation(population);
    }

    /*! Configura a observação de progresso do algoritmo. */
    void observe_solution_progress(algorithm& brkga) const {
        int last_update_iteration = -100;
        brkga.addNewSolutionObserver(
            [&last_update_iteration](
                const BRKGA::AlgorithmStatus& status) -> bool {
                if (status.current_iteration - last_update_iteration >= 100) {
                    std::cout
                        << "Improved best individual: " << status.best_fitness
                        << ". Iteration " << status.current_iteration
                        << ". Current time: " << status.current_time
                        << std::endl;
                    last_update_iteration = status.current_iteration;
                }
                return true;
            });
    }

    /*! Função de perturbação para as soluções do algoritmo. */
    template <typename URBG>
    decltype(BRKGA::BrkgaParams::custom_shaking)
    shaking_function(URBG&& rng, next_fit_decoder& decoder) const {
        return [&](double lower_bound, double upper_bound, auto& populations,
                   auto& shaken) {
            std::uniform_real_distribution<> uniform(0, 1);

            double chance =
                std::uniform_real_distribution<>(lower_bound, upper_bound)(rng);

            std::cout << "Shuffling levels and randomly changing order of "
                         "rectangles with probability "
                      << chance << std::endl;

            for (unsigned i = 0; i < populations.size(); i++) {
                auto& population = populations[i]->chromosomes;
                for (unsigned j = 0; j < population.size(); j++) {
                    auto& chromosome = population[j];

                    // Embaralha os níveis da solução.
                    solution_t solution = decoder.rebuild(chromosome);
                    for (auto& level : solution) {
                        std::shuffle(level.begin(), level.end(), rng);
                    }

                    // Retorna a solução para a representação como cromossomo e
                    // modifica genes de forma aleatória.
                    bool change = false;
                    chromosome = encode(solution);
                    for (auto& gene : chromosome) {
                        if (uniform(rng) <= chance) {
                            gene = uniform(rng);
                            change = true;
                        }
                    }
                    if (change) {
                        shaken.push_back({i, j});
                    }
                }
            }
        };
    }

    const instance_t& m_instance;
    const std::vector<solution_t>& m_initial;
};

} // namespace improvement

class runner {
  public:
    struct config {
        size_t random_seed;
        bool brkga_enabled;
        std::string brkga_config;
        size_t first_fit_samples;
        double first_fit_random_deviations;
        size_t best_fit_samples;
        double best_fit_random_deviations;
        std::string output;
    };

  private:
    const instance_t& m_instance;
    const config& m_config;

    double m_weight_stddev;
    double m_height_stddev;

  public:
    runner(const instance_t& instance, const config& conf)
        : m_instance(instance), m_config(conf) {

        // Computa o desvio padrão do peso e altura dos retângulos, usados para
        // adicionar perturbações aleatórias nas instâncias para as heurísticas
        // aleatorizadas.
        double acc_weight = 0.0;
        double acc_height = 0.0;
        for (auto& rect : instance.rects) {
            acc_weight += rect.weight;
            acc_height += rect.height;
        }

        double mean_weight = acc_weight / instance.rects.size();
        double mean_height = acc_height / instance.rects.size();

        acc_weight = 0.0;
        acc_height = 0.0;
        for (auto& rect : instance.rects) {
            acc_weight += std::pow(rect.weight - mean_weight, 2);
            acc_height += std::pow(rect.height - mean_height, 2);
        }

        m_weight_stddev = std::sqrt(acc_weight / instance.rects.size());
        m_height_stddev = std::sqrt(acc_height / instance.rects.size());

        std::cout << "Weight standard deviation: " << m_weight_stddev
                  << std::endl;
        std::cout << "Height standard deviation: " << m_height_stddev
                  << std::endl;
    }

    /**
     * Gera soluções para a instância do problema utilizando a heurística de
     * first-fit aleatorizada.
     *
     * Devolve a melhor solução dentre todas as soluções geradas.
     */
    template <class URBG>
    solution_t run_first_fit(URBG&& rng, size_t samples,
                             std::vector<solution_t>& solutions) {
        std::normal_distribution<> noise(
            0.0, m_config.first_fit_random_deviations * m_weight_stddev);
        solution_t best;
        cost_type best_cost = -1;
        for (size_t i = 0; i < samples; i++) {
            auto solution = heuristics::constructive::
                randomized_first_fit_decreasing_density(m_instance, rng, noise);
            solutions.push_back(solution);

            cost_type cost = m_instance.cost(solution);
            if (best_cost < 0 || cost < best_cost) {
                best = solution;
                best_cost = cost;
            }
        }
        return best;
    }

    /**
     * Gera soluções para a instância do problema utilizando a heurística de
     * best-fit aleatorizada.
     *
     * Devolve a melhor solução dentre todas as soluções geradas.
     */
    template <class URBG>
    solution_t run_best_fit(URBG&& rng, size_t samples,
                            std::vector<solution_t>& solutions) {
        std::normal_distribution<> noise(
            0.0, m_config.best_fit_random_deviations * m_height_stddev);
        solution_t best;
        cost_type best_cost = -1;
        for (size_t i = 0; i < samples; i++) {
            auto solution =
                heuristics::constructive::randomized_best_fit_increasing_height(
                    m_instance, rng, noise);
            solutions.push_back(solution);

            cost_type cost = m_instance.cost(solution);
            if (best_cost < 0 || cost < best_cost) {
                best = solution;
                best_cost = cost;
            }
        }
        return best;
    }

    /**
     * Melhora soluções utilizando o algoritmo BRKGA-MP-IPR.
     *
     * Devolve a melhor solução obtida.
     */
    template <class URBG>
    solution_t run_brkga(URBG&& rng, const BRKGA::BrkgaParams& brkga_params,
                         const BRKGA::ControlParams& control_params,
                         std::vector<solution_t>&& initial) {
        std::shuffle(initial.begin(), initial.end(), rng);
        return heuristics::improvement::brkga_mp_ipr(m_instance, initial)
            .run(rng, brkga_params, control_params, 24);
    }

    /*! Executa as heurísticas. */
    void run() {
        std::ofstream out;
        std::minstd_rand rng(m_config.random_seed);

        out << std::fixed << std::setprecision(3);
        out.open(m_config.output + "/instance.txt");
        io::print_instance(out, m_instance);
        out.close();

        std::vector<solution_t> initial;
        initial.reserve(m_config.first_fit_samples + m_config.best_fit_samples);

        out.open(m_config.output + "/first-fit.txt");
        out << "[Randomized first-fit decreasing density heuristic solution]"
            << std::endl;
        auto first_fit_solution =
            run_first_fit(rng, m_config.first_fit_samples, initial);
        io::print_solution(out, m_instance, first_fit_solution);
        out.close();
        render::render_solution(m_instance, first_fit_solution,
                                m_config.output + "/first-fit.png");

        out.open(m_config.output + "/best-fit.txt");
        out << "[Randomized best-fit increasing height heuristic solution]"
            << std::endl;
        auto best_fit_solution =
            run_best_fit(rng, m_config.best_fit_samples, initial);
        io::print_solution(out, m_instance, best_fit_solution);
        out.close();
        render::render_solution(m_instance, best_fit_solution,
                                m_config.output + "/best-fit.png");

        if (m_config.brkga_enabled) {
            out.open(m_config.output + "/brkga.txt");
            out << "[BRKGA]" << std::endl;
            auto [brkga_params, control_params] =
                BRKGA::readConfiguration(m_config.brkga_config);

            // Garante que cada população seja composta inicialmente por, no
            // máximo, 50% de soluções heurísticas.
            brkga_params.population_size =
                std::max(brkga_params.population_size,
                         unsigned(m_config.best_fit_samples +
                                  m_config.first_fit_samples) *
                             2 / brkga_params.num_independent_populations);

            auto brkga_solution = run_brkga(rng, brkga_params, control_params,
                                            std::move(initial));
            io::print_solution(out, m_instance, brkga_solution);
            out.close();
            render::render_solution(m_instance, brkga_solution,
                                    m_config.output + "/brkga.png");
        }
    }
};

} // namespace strip_packing::heuristics

#endif // STRIP_PACKING_HEURISTICS_HPP
