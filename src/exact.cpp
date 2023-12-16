#include <cmath>
#include <cstddef>
#include <fstream>
#include <random>
#include <stdexcept>

#include <strip_packing.hpp>
#include <strip_packing/io.hpp>
#include <strip_packing/render.hpp>

#include <argparse/argparse.hpp>

#include <brkga_mp_ipr/brkga_mp_ipr.hpp>

using namespace strip_packing;

struct columns_t {
    const instance_t& instance;
    size_t size;

    std::vector<std::vector<int>> incidence;

    columns_t(const instance_t& instance) : instance(instance) {}

    columns_t(const instance_t& instance, const solution_t& initial)
        : columns_t(instance) {
        const auto N = instance.rects.size();

        size = initial.size() + 1;

        incidence.resize(N);
        for (size_t i = 0; i < N; i++) {
            incidence[i].resize(size);
        }
        for (size_t k = 0; k < initial.size(); k++) {
            for (size_t i : initial[k]) {
                incidence[i][k] = true;
            }
        }
    }

    std::vector<int> operator[](size_t j) const {
        std::vector<int> column(instance.rects.size());
        for (int i = 0; i < column.size(); i++) {
            column.push_back(incidence[i][j]);
        }
        return column;
    }

    void add(std::vector<int> x) {
        for (size_t i = 0; i < instance.rects.size(); i++) {
            incidence[i].push_back(x[i]);
        }

        size++;
    }
};

class restricted_master_problem {
  private:
    GRBModel* rmp;

    const instance_t& instance;
    columns_t columns;

    dim_type MAX_HEIGHT;
    std::vector<dim_type> SORTED_HEIGHTS;

    std::vector<std::vector<GRBVar>> lambda;
    std::vector<GRBVar> item_base;
    std::vector<GRBVar> level_height;

    std::vector<GRBConstr> exact_cover_constraints;
    std::vector<std::vector<GRBConstr>> item_base_constraints;
    std::vector<std::vector<GRBConstr>> level_height_constraints;
    std::vector<GRBConstr> convexity_constraints;

    std::vector<std::vector<bool>> usable;

    void rebuild_model(bool reset_model = false) {
        if (reset_model) {
            auto* aux = rmp;
            rmp = new GRBModel(rmp->getEnv());
            delete aux;
        }

        static char var_name[32];

        const auto N = instance.rects.size();
        const auto K = columns.size;

        usable.resize(N);
        for (size_t j = 0; j < N; j++) {
            usable[j].resize(K);
            for (size_t k = 0; k < K; k++) {
                usable[j][k] = true;
                for (size_t i = 0; i < N; i++) {
                    if (fixed[i][j] == 0) {
                        continue;
                    }
                    if (columns.incidence[i][k] != (fixed[i][j] > 0)) {
                        usable[j][k] = false;
                        break;
                    }
                }
            }
        }

        // Variáveis
        lambda.resize(N);
        for (size_t j = 0; j < N; j++) {
            lambda[j].resize(K);
            for (size_t k = 0; k < K; k++) {
                std::snprintf(var_name, sizeof(var_name), "lambda[%zu][%zu]", j,
                              k);
                lambda[j][k] = rmp->addVar(0.0, usable[j][k], 0.0,
                                           GRB_CONTINUOUS, var_name);
            }
        }

        item_base.resize(N);
        for (size_t i = 0; i < N; i++) {
            std::snprintf(var_name, sizeof(var_name), "item_base[%zu]", i);
            item_base[i] =
                rmp->addVar(0.0, MAX_HEIGHT, instance.rects[i].weight,
                            GRB_CONTINUOUS, var_name);
        }

        level_height.resize(N);
        for (size_t i = 0; i < N; i++) {
            std::snprintf(var_name, sizeof(var_name), "level_height[%zu]", i);
            level_height[i] = rmp->addVar(0.0, SORTED_HEIGHTS[N - 1], 0.0,
                                          GRB_CONTINUOUS, var_name);
        }

        rmp->update();

        // Restrições
        exact_cover_constraints.resize(N);
        for (size_t i = 0; i < N; i++) {
            GRBLinExpr expr;
            for (size_t j = 0; j < N; j++) {
                for (size_t k = 0; k < K; k++) {
                    expr += columns.incidence[i][k] * lambda[j][k];
                }
            }
            std::snprintf(var_name, sizeof(var_name), "exact_cover[%zu]", i);
            exact_cover_constraints[i] = rmp->addConstr(expr == 1, var_name);
        }

        item_base_constraints.resize(N);
        for (size_t i = 0; i < N; i++) {
            item_base_constraints[i].resize(N);

            dim_type max_total_height = 0;
            for (size_t j = 0; j < N; j++) {
                GRBLinExpr x_expr;
                for (size_t k = 0; k < K; k++) {
                    if (columns.incidence[i][k]) {
                        x_expr += lambda[j][k];
                    }
                }

                GRBLinExpr level_height_expr;
                for (size_t l = 0; l < j; l++) {
                    level_height_expr += level_height[l];
                }

                std::snprintf(var_name, sizeof(var_name),
                              "item_base_slack[%zu][%zu]", i, j);

                std::snprintf(var_name, sizeof(var_name), "item_base[%zu][%zu]",
                              i, j);
                item_base_constraints[i][j] = rmp->addConstr(
                    item_base[i] + max_total_height * (1 - x_expr) -
                            level_height_expr >=
                        0,
                    var_name);

                max_total_height += SORTED_HEIGHTS[N - j - 1];
            }
        }

        level_height_constraints.resize(N);
        for (size_t i = 0; i < N; i++) {
            level_height_constraints[i].resize(N);
            for (size_t j = 0; j < N; j++) {
                GRBLinExpr x_expr;
                for (size_t k = 0; k < K; k++) {
                    if (columns.incidence[i][k]) {
                        x_expr += lambda[j][k];
                    }
                }

                std::snprintf(var_name, sizeof(var_name),
                              "level_height[%zu][%zu]", i, j);
                level_height_constraints[i][j] = rmp->addConstr(
                    level_height[j] - instance.rects[i].height * x_expr >= 0,
                    var_name);
            }
        }

        convexity_constraints.resize(N);
        for (size_t j = 0; j < N; j++) {
            GRBLinExpr expr;
            for (size_t k = 0; k < K; k++) {
                expr += lambda[j][k];
            }
            std::snprintf(var_name, sizeof(var_name), "convexity[%zu]", j);
            convexity_constraints[j] = rmp->addConstr(expr == 1, var_name);
        }
    }

    bool generate_column() {
        static char var_name[32];

        const auto N = instance.rects.size();
        const auto values = solution();

        dim_type max_total_height = 0;
        for (size_t j = 0; j < N; j++) {
            GRBModel pricing(rmp->getEnv());

            std::vector<GRBVar> x(N);
            for (size_t i = 0; i < N; i++) {
                double lb = 0.0, ub = 1.0;
                if (fixed[i][j] > 0) {
                    lb = 1.0;
                } else if (fixed[i][j] < 0) {
                    ub = 0.0;
                }

                double cost =
                    -exact_cover_constraints[i].get(GRB_DoubleAttr_Pi) +
                    +instance.rects[i].height *
                        level_height_constraints[i][j].get(GRB_DoubleAttr_Pi) +
                    max_total_height *
                        item_base_constraints[i][j].get(GRB_DoubleAttr_Pi);

                std::snprintf(var_name, sizeof(var_name), "x[%zu]", i);
                x[i] = pricing.addVar(lb, ub, cost, GRB_BINARY, var_name);
            }

            double u_a = 0.0;
            for (size_t i = 0; i < N; i++) {
                for (size_t l = j + 1; l < N; l++) {
                    u_a += item_base_constraints[i][l].get(GRB_DoubleAttr_Pi);
                }
            }

            pricing.update();

            GRBLinExpr length_expr;
            for (size_t i = 0; i < N; i++) {
                length_expr += instance.rects[i].length * x[i];
            }
            pricing.addConstr(length_expr <= instance.recipient_length,
                              "packing");

            max_total_height += SORTED_HEIGHTS[N - j - 1];
            pricing.update();

            double u_0 = convexity_constraints[j].get(GRB_DoubleAttr_Pi);
            pricing.set(GRB_DoubleAttr_ObjCon, -u_0);

            pricing.write("pr.lp");
            pricing.optimize();

            if (pricing.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                continue;
            }

            pricing.write("pr.sol");

            double reduced_cost = pricing.get(GRB_DoubleAttr_ObjVal);

            if (reduced_cost < -1e-4) {
                std::vector<int> column(N);
                for (size_t i = 0; i < N; i++) {
                    column[i] = x[i].get(GRB_DoubleAttr_X) > 0.5;
                }
                columns.add(column);
                rebuild_model(true);
                return true;
            }
        }
        return false;
    }

  public:
    std::vector<std::vector<int>> fixed;

    restricted_master_problem(const GRBEnv& env, const instance_t& instance,
                              const columns_t& columns,
                              const std::vector<std::vector<int>>& fixed)
        : rmp(new GRBModel(env)), instance(instance), columns(columns),
          fixed(fixed) {
        MAX_HEIGHT = 0;
        for (auto rect : instance.rects) {
            MAX_HEIGHT += rect.height;
            SORTED_HEIGHTS.push_back(rect.height);
        }
        std::sort(SORTED_HEIGHTS.begin(), SORTED_HEIGHTS.end());

        rebuild_model();
    }

    ~restricted_master_problem() { delete rmp; }

    bool optimize() {
        do {
            rmp->write("rmp.lp");
            rmp->optimize();
            if (rmp->get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                return false;
            }
            rmp->write("rmp.sol");
        } while (generate_column());
        return true;
    }

    const std::vector<std::vector<double>> solution() const {
        const auto N = instance.rects.size();
        std::vector<std::vector<double>> x(N);
        for (size_t i = 0; i < N; i++) {
            x[i].resize(N);
        }
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                x[i][j] = 0;
                for (size_t k = 0; k < columns.size; k++) {
                    x[i][j] += columns.incidence[i][k] *
                               lambda[j][k].get(GRB_DoubleAttr_X);
                }
            }
        }
        return x;
    }

    bool viable() const { return rmp->get(GRB_IntAttr_Status) == GRB_OPTIMAL; }

    double cost() const { return rmp->get(GRB_DoubleAttr_ObjVal); }

    GRBModel* model() { return rmp; }

    restricted_master_problem* fixing(size_t i, size_t j, int value) {
        std::swap(fixed[i][j], value);

        auto* subproblem = new restricted_master_problem(
            rmp->getEnv(), instance, columns, fixed);
        std::swap(fixed[i][j], value);
        return subproblem;
    }
};

/**
 * Resolve uma instância do problema de forma exata com geração de colunas.
 */
solution_t solve_with_column_generation(const instance_t& instance,
                                        const solution_t& initial) {
    const auto N = instance.rects.size();

    GRBEnv env;
    env.set("OutputFlag", "0");

    using problem_t = restricted_master_problem*;
    using node_t = std::pair<double, problem_t>;

    std::vector<std::vector<int>> fixed(N);
    for (size_t i = 0; i < N; i++) {
        fixed[i].resize(N);
    }

    problem_t root_problem = new restricted_master_problem(
        env, instance, columns_t(instance, initial), fixed);
    if (!root_problem->optimize()) {
        throw std::logic_error("No valid solution found");
    }

    node_t root = std::make_pair(root_problem->cost(), root_problem);

    std::priority_queue<node_t, std::vector<node_t>, std::greater<node_t>> q;
    q.push(root);

    double best_cost = instance.cost(initial);
    problem_t best = nullptr;

    int explored = 0;
    while (!q.empty()) {
        auto [cost, problem] = q.top();
        q.pop();
        explored++;

        std::cout << "Explored: " << explored << " Open: " << q.size()
                  << " Best Cost: " << best_cost << " BestBd: " << cost
                  << " Gap: " << 100 * (best_cost - cost) / best_cost << "%"
                  << std::endl;

        if (best != nullptr || cost >= best_cost) {
            delete problem;
            continue;
        }

        const auto x = problem->solution();
        bool integer = true;

        for (size_t j = 0; integer && j < N; j++) {
            for (size_t i = 0; integer && i < N; i++) {
                if (x[i][j] <= 1e-4 || x[i][j] >= 1 - 1e-4) {
                    continue;
                }
                integer = false;

                auto* subproblem0 = problem->fixing(i, j, -1);
                subproblem0->optimize();
                if (subproblem0->viable()) {
                    q.push(std::make_pair(subproblem0->cost(), subproblem0));
                }

                auto* subproblem1 = problem->fixing(i, j, 1);
                subproblem1->optimize();
                if (subproblem1->viable()) {
                    q.push(std::make_pair(subproblem1->cost(), subproblem1));
                }
            }
        }

        if (integer && cost < best_cost) {
            best_cost = cost;
            std::swap(best, problem);
        }

        delete problem;
    }

    if (best == nullptr) {
        return initial;
    }

    best->model()->write("best.lp");
    best->model()->write("best.sol");

    const auto x = best->solution();
    delete best;

    solution_t solution;
    for (size_t j = 0; j < N; j++) {
        instance_t::rect_subset level;
        for (size_t i = 0; i < N; i++) {
            if (x[i][j] > 0.5) {
                level.push_back(i);
            }
        }

        if (!level.empty()) {
            solution.push_back(level);
        }
    }

    return solution;
}

/*! Ponto de entrada. */
int main(int argc, char** argv) {
    argparse::ArgumentParser program("mc859-strip-packing-heuristics");

    program.add_argument("-s", "--seed")
        .metavar("N")
        .help("seed for the random number generator.")
        .scan<'u', unsigned>();

    program.add_argument("-o", "--output")
        .metavar("DIR")
        .default_value<std::string>(".")
        .help("output directory.");

    program.add_argument("--no-brkga")
        .default_value(false)
        .implicit_value(true)
        .help("disable BRKGA improvement heuristic.")
        .nargs(0);

    program.add_argument("--brkga-config")
        .default_value<std::string>("brkga.conf")
        .metavar("FILE")
        .help("BRKGA configuration file.");

    program.add_argument("--first-fit")
        .default_value<unsigned>(500)
        .metavar("N")
        .help("number of random samples of the first fit heuristic.")
        .scan<'u', unsigned>();

    program.add_argument("--first-fit-deviations")
        .default_value<double>(.25)
        .metavar("N")
        .help("standard deviations to use for randomization of the first fit "
              "heuristic.")
        .scan<'g', double>();

    program.add_argument("--best-fit")
        .default_value<unsigned>(500)
        .metavar("N")
        .help("number of random samples of the best fit heuristic.")
        .scan<'u', unsigned>();

    program.add_argument("--best-fit-deviations")
        .default_value<double>(.25)
        .metavar("N")
        .help("standard deviations to use for randomization of the best fit "
              "heuristic.")
        .scan<'g', double>();

    program.add_argument("file").help("instance file name.");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    size_t seed;
    if (auto s = program.present<unsigned>("-s")) {
        seed = *s;
    } else {
        std::random_device rd;
        seed = rd();
    }

    instance_t instance;
    {
        auto filename = program.get("file");
        std::ifstream file(filename);
        instance = io::read_instance(file);
    }

    std::minstd_rand rng(seed);
    auto heuristic_solution =
        heuristics::constructive::randomized_first_fit_decreasing_density(
            instance, rng);

    std::cout << "[Heuristic solution]" << std::endl;
    io::print_solution(std::cout, instance, heuristic_solution);
    
    GRBEnv env;
    // env.set(GRB_DoubleParam_Cutoff, instance.cost(heuristic_solution));
    
    auto solution = exact::solve(env, instance);
    // auto solution = solve_with_column_generation(instance, heuristic_solution);
    std::cout << "[Optimal solution]" << std::endl;
    io::print_solution(std::cout, instance, solution);
    render::render_solution(instance, solution, "exact.png");
}
