#ifndef STRIP_PACKING_EXACT_HPP
#define STRIP_PACKING_EXACT_HPP

#include "defs.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <queue>
#include <vector>

#include <gurobi_c++.h>

namespace strip_packing::exact {

struct mip_variables {
    const size_t N;

    std::vector<std::vector<GRBVar>> incidence;
    std::vector<GRBVar> level_used;
    std::vector<GRBVar> level_height;
    std::vector<GRBVar> item_base;

    mip_variables() = delete;
    mip_variables(const mip_variables&) = delete;

    mip_variables(GRBModel& model, const instance_t& instance)
        : N(instance.rects.size()), incidence(N), level_used(N),
          level_height(N), item_base(N) {
        dim_type min_height = std::numeric_limits<dim_type>::max(),
                 max_height = 0.0;
        dim_type max_total_height = 0.0;
        for (const auto& rect : instance.rects) {
            min_height = std::min(min_height, rect.height);
            max_height = std::max(max_height, rect.height);
            max_total_height += rect.height;
        }

        char var_name[32];

        for (size_t i = 0; i < N; i++) {
            std::snprintf(var_name, sizeof(var_name), "level_height[%zu]", i);
            level_height[i] = model.addVar(min_height, max_height, 0.0,
                                           GRB_CONTINUOUS, var_name);

            std::snprintf(var_name, sizeof(var_name), "level_used[%zu]", i);
            level_used[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, var_name);

            std::snprintf(var_name, sizeof(var_name), "item_base[%zu]", i);
            item_base[i] =
                model.addVar(0.0, max_total_height, instance.rects[i].weight,
                             GRB_CONTINUOUS, var_name);

            incidence[i].resize(N);
            for (size_t j = 0; j < N; j++) {
                std::snprintf(var_name, sizeof(var_name), "incidence[%zu,%zu]",
                              i, j);
                incidence[i][j] =
                    model.addVar(0.0, 1.0, 0.0, GRB_BINARY, var_name);
            }
        }
    }
};

/**
 * Resolve uma instância do problema de forma exata.
 */
static inline solution_t solve(GRBEnv env, const instance_t& instance) {
    GRBModel model(&env);

    mip_variables vars(model, instance);

    char constr_name[64];

    std::vector<dim_type> sorted_heights;
    for (auto rect : instance.rects) {
        sorted_heights.push_back(rect.height);
    }
    std::sort(sorted_heights.begin(), sorted_heights.end());

    // Cobertura exata
    size_t N = instance.rects.size();
    for (size_t i = 0; i < N; i++) {
        GRBLinExpr expr;
        for (size_t j = 0; j < N; j++) {
            expr += vars.incidence[i][j];
        }
        std::snprintf(constr_name, sizeof(constr_name), "exact_cover[%zu]", i);
        model.addConstr(expr == 1, constr_name);
    }

    // Indicadores de uso dos níveis
    for (size_t j = 0; j < N; j++) {
        for (size_t i = 0; i < N; i++) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "level_used[%zu][%zu]", i, j);
            model.addConstr(vars.level_used[j] >= vars.incidence[i][j],
                            constr_name);
        }

        GRBLinExpr expr;
        for (size_t i = 0; i < N; i++) {
            expr += vars.incidence[i][j];
        }
        model.addConstr(expr >= vars.level_used[j]);

        if (j > 0) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "prev_level_used[%zu]", j);
            model.addConstr(vars.level_used[j - 1] >= vars.level_used[j],
                            constr_name);
        }
    }

    // Empacotamento nos níveis
    for (size_t j = 0; j < N; j++) {
        GRBLinExpr expr;
        for (size_t i = 0; i < N; i++) {
            expr += instance.rects[i].length * vars.incidence[i][j];
        }
        std::snprintf(constr_name, sizeof(constr_name), "level_packing[%zu]",
                      j);
        model.addConstr(expr <= instance.recipient_length, constr_name);
    }

    // Altura dos níveis
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "level_height[%zu][%zu]", i, j);
            model.addConstr(vars.level_height[j] >=
                                instance.rects[i].height * vars.incidence[i][j],
                            constr_name);
        }
    }

    // Altura da base dos retângulos
    dim_type max_total_height = 0, min_total_height = 0;
    for (size_t j = 0; j < N; j++) {
        GRBLinExpr level_base_expr;
        for (size_t k = 0; k + 1 <= j; k++) {
            level_base_expr += vars.level_height[k];
        }

        std::snprintf(constr_name, sizeof(constr_name), "level_base_lb[%zu]",
                      j);
        model.addConstr(level_base_expr >= min_total_height, constr_name);

        for (size_t i = 0; i < N; i++) {
            std::snprintf(constr_name, sizeof(constr_name),
                          "item_base_level_base[%zu][%zu]", i, j);
            model.addConstr(vars.item_base[i] >=
                                level_base_expr -
                                    max_total_height *
                                        (1 - vars.incidence[i][j]),
                            constr_name);
        }

        max_total_height += sorted_heights[sorted_heights.size() - 1 - j];
        min_total_height += sorted_heights[j];
    }

    model.write("model.lp");
    model.optimize();
    model.write("solution.sol");

    solution_t solution;
    for (size_t j = 0; j < N; j++) {
        instance_t::rect_subset level;
        for (size_t i = 0; i < N; i++) {
            if (vars.incidence[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                level.push_back(i);
            }
        }
        
        if (level.empty()) {
            break;
        }
        
        solution.push_back(level);
    }

    return solution;
}

} // namespace strip_packing::exact

#endif // STRIP_PACKING_EXACT_HPP
