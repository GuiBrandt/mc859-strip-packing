#ifndef STRIP_PACKING_EXACT_HPP
#define STRIP_PACKING_EXACT_HPP

#include "defs.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <queue>
#include <vector>

#include <gurobi_c++.h>
#include <lemon/list_graph.h>

namespace strip_packing::exact {

struct mip_variables {
    const size_t N;

    std::vector<std::vector<GRBVar>> item_level;
    std::vector<std::vector<GRBVar>> prev_level;
    std::vector<std::vector<GRBVar>> weight_flow;

    mip_variables() = delete;
    mip_variables(const mip_variables&) = delete;

    mip_variables(GRBModel& model, const instance_t& instance)
        : N(instance.rects.size()), item_level(N), prev_level(N),
          weight_flow(N) {
        char var_name[32];

        for (size_t i = 0; i < N; i++) {
            item_level[i].resize(N);
            for (size_t j = 0; j < N; j++) {
                std::snprintf(var_name, sizeof(var_name), "item_level[%zu,%zu]",
                              i, j);
                item_level[i][j] =
                    model.addVar(0.0, 1.0, 0.0, GRB_BINARY, var_name);
            }

            prev_level[i].resize(N + 1);
            for (size_t j = 0; j < N + 1; j++) {
                std::snprintf(var_name, sizeof(var_name), "prev_level[%zu,%zu]",
                              i, j);
                prev_level[i][j] =
                    model.addVar(0.0, i != j, 0.0, GRB_BINARY, var_name);
            }

            weight_flow[i].resize(N + 1);
            for (size_t j = 0; j < N + 1; j++) {
                std::snprintf(var_name, sizeof(var_name),
                              "weight_flow[%zu][%zu]", i, j);
                weight_flow[i][j] = model.addVar(
                    0.0, GRB_INFINITY, j < N ? instance.rects[j].height : 0.0,
                    GRB_CONTINUOUS, var_name);
            }
        }

        model.update();
    }
};

class tour_elimination : public GRBCallback {
  private:
    const instance_t& instance;
    const mip_variables& vars;

  protected:
    void callback() override {
        switch (where) {
        case GRB_CB_MIPSOL: {
            using Graph = lemon::ListGraph;

            const auto N = instance.rects.size();

            Graph G;
            std::vector<Graph::Node> nodes;
            for (size_t i = 0; i < N + 1; i++) {
                nodes.push_back(G.addNode());
            }

            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < N + 1; i++) {
                    if (i == j)
                        continue;
                    if (getSolution(vars.item_level[i][j]) +
                            getSolution(vars.prev_level[i][j]) >
                        1 - 1e-5) {
                        G.addEdge(nodes[i], nodes[j]);
                    }
                }
            }
            break;
        }
        case GRB_CB_MIPNODE: {
            if (getIntInfo(GRB_CB_MIPNODE_STATUS) != GRB_OPTIMAL) {
                break;
            }
        }
        }
    }

  public:
    tour_elimination(const instance_t& instance, const mip_variables& vars)
        : instance(instance), vars(vars) {}
};

/**
 * Resolve uma instância do problema de forma exata.
 */
static inline solution_t solve(GRBEnv env, const instance_t& instance) {
    solution_t solution;

    GRBModel model(&env);

    mip_variables vars(model, instance);

    char constr_name[64];

    // Cobertura exata
    size_t N = instance.rects.size();
    for (size_t i = 0; i < N; i++) {
        GRBLinExpr expr;
        for (size_t j = 0; j < N; j++) {
            expr += vars.item_level[i][j];
        }
        std::snprintf(constr_name, sizeof(constr_name), "exact_cover[%zu]", i);
        model.addConstr(expr >= 1, constr_name);
    }

    // Empacotamento nos níveis
    for (size_t j = 0; j < N; j++) {
        GRBLinExpr expr;
        for (size_t i = 0; i < N; i++) {
            expr += instance.rects[i].length * vars.item_level[i][j];
        }
        std::snprintf(constr_name, sizeof(constr_name), "level_packing[%zu]",
                      j);
        model.addConstr(expr <= instance.recipient_length * vars.item_level[j][j], constr_name);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            if (i == j) {
                continue;
            }

            std::snprintf(constr_name, sizeof(constr_name),
                          "level_height[%zu][%zu]", i, j);
            model.addConstr(
                instance.rects[i].height * vars.item_level[i][j] <=
                    instance.rects[j].height * vars.item_level[j][j],
                constr_name);
        }
    }

    for (size_t j = 0; j < N; j++) {
        for (size_t i = 0; i < N; i++) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "level_used[%zu][%zu]", i, j);
            model.addConstr(vars.item_level[i][j] <= vars.item_level[j][j],
                            constr_name);
        }
    }

    // Fluxo
    GRBEnv env2;
    GRBModel knapsack(env2);
    for (size_t i = 0; i < N; i++) {
        knapsack.addVar(0.0, 1.0, instance.rects[i].weight, GRB_BINARY);
    }

    cost_type max_weight = 0;

    for (size_t i = 0; i < N; i++) {
        GRBLinExpr flow_in_expr;
        for (size_t j = 0; j < N; j++) {
            flow_in_expr += instance.rects[j].weight * vars.item_level[j][i];
            flow_in_expr += vars.weight_flow[j][i];
        }

        GRBLinExpr flow_out_expr;
        for (size_t j = 0; j < N + 1; j++) {
            flow_out_expr += vars.weight_flow[i][j];
        }

        for (size_t j = 0; j < N + 1; j++) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "flow_used_edges[%zu][%zu]", i, j);
            model.addConstr(vars.weight_flow[i][j] <=
                                max_weight * vars.prev_level[i][j],
                            constr_name);
        }
        std::snprintf(constr_name, sizeof(constr_name), "node_flow[%zu]", i);
        model.addConstr(flow_in_expr == flow_out_expr, constr_name);
    }

    // Topologia
    GRBLinExpr arcs_expr;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N + 1; j++) {
            if (i != j && j < N) {
                arcs_expr += vars.item_level[i][j];
            }
            arcs_expr += vars.prev_level[i][j];
        }
    }
    model.addConstr(arcs_expr == N);

    for (size_t i = 0; i < N + 1; i++) {
        GRBLinExpr next_expr;
        for (size_t j = 0; j < N; j++) {
            next_expr += vars.prev_level[j][i];
        }
        std::snprintf(constr_name, sizeof(constr_name), "in_one[%zu]", i);
        if (i < N) {
            model.addConstr(next_expr <= vars.item_level[i][i], constr_name);
        } else {
            model.addConstr(next_expr == 1, constr_name);
        }
    }

    for (size_t i = 0; i < N; i++) {
        GRBLinExpr prev_expr;
        for (size_t j = 0; j < N + 1; j++) {
            prev_expr += vars.prev_level[i][j];
        }
        std::snprintf(constr_name, sizeof(constr_name), "out_one[%zu]", i);
        model.addConstr(prev_expr == vars.item_level[i][i], constr_name);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = i + 1; j < N; j++) {
            model.addConstr(vars.item_level[i][j] + vars.item_level[j][i] +
                                vars.prev_level[i][j] + vars.prev_level[j][i] <=
                            1);
        }
    }

    model.update();
    model.write("model.lp");

    auto relaxed = model.relax();
    relaxed.optimize();
    relaxed.write("relaxed.sol");

    std::cout << "digraph D {" << std::endl;
    std::ios init(NULL);
    init.copyfmt(std::cout);
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N + 1; j++) {
            double x = relaxed
                           .getVarByName(vars.prev_level[i][j].get(
                               GRB_StringAttr_VarName))
                           .get(GRB_DoubleAttr_X);
            if (x < 1e-5) {
                continue;
            }
            std::cout << "\t" << i << " -> " << j << " [color=\"#ff0000"
                      << std::hex << int(x * 255) << "\",weight=" << x << "];"
                      << std::endl;
            std::cout.copyfmt(init);
        }
        for (size_t j = 0; j < N; j++) {
            double x = relaxed
                           .getVarByName(vars.item_level[i][j].get(
                               GRB_StringAttr_VarName))
                           .get(GRB_DoubleAttr_X);
            if (x < 1e-5) {
                continue;
            }
            std::cout << "\t" << i << " -> " << j << " [color=\"#0000ff"
                      << std::hex << int(x * 255) << "\",weight=" << x << "];"
                      << std::endl;
            std::cout.copyfmt(init);
        }
    }
    std::cout << "}" << std::endl;

    model.optimize();
    model.write("model.sol");

    size_t i = N;
    for (size_t j;; i = j) {
        for (j = 0; j < N; j++) {
            if (vars.prev_level[j][i].get(GRB_DoubleAttr_X) > 0.5) {
                break;
            }
        }

        if (j == N) {
            break;
        }

        instance_t::rect_subset level;
        for (size_t k = 0; k < N; k++) {
            if (vars.item_level[k][j].get(GRB_DoubleAttr_X) > 0.5) {
                level.push_back(k);
            }
        }

        solution.push_back(level);
    }

    std::cout << "digraph D {" << std::endl;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N + 1; j++) {
            if (vars.prev_level[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                std::cout << "\t" << i << " -> " << j << " [color=red];"
                          << std::endl;
            }
        }
        for (size_t j = 0; j < N; j++) {
            if (vars.item_level[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                std::cout << "\t" << i << " -> " << j << " [color=blue];"
                          << std::endl;
            }
        }
    }
    std::cout << "}" << std::endl;

    return solution;
}

} // namespace strip_packing::exact

#endif // STRIP_PACKING_EXACT_HPP
