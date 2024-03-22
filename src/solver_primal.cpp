#include <algorithm>
#include <cmath>
#include <vector>

#include "solver.hpp"

#include <gurobi_c++.h>

#include "formulation.hpp"
#include "pricing.hpp"
#include "utils.hpp"

solver::solver(formulation& orig)
    : env(environment::get_env())
    , form(orig)
    , grb(env)
    , constrs(form.get_graph().get_n())
{
    LOG_SCOPE_F(INFO, "Initializing solver.");
    DCHECK_F(form.get_graph().get_n() > 0, "Graph is empty.");
    DCHECK_F(form.check_all(), "Formulation is not valid.");

    // === Create the model ================================================
    grb.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    grb.set(GRB_IntParam_Presolve, 0);
    grb.set(GRB_IntParam_Method, 0);

    // Each independent set is a variable
    for_indep_set(form, s) {
        vars[s] = grb.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS);
    }

    grb.update();

    // For each vertice, it has to be covered by at least one set
    for_nodes(form.get_graph(), v) {
        GRBLinExpr c = 0;
        for_indep_set_with(form, v, set) {
            c += vars[set];
        }
        // check if something has been added to c
        DCHECK_F(c.size() > 0, "No set covers node %d.", v);

        HANDLE_GRB_EXCEPTION(constrs[v] = grb.addConstr(c >= 1.0));
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)vars.size());
}

void solver::add_variable(const node_set& set)
{
    form.add_indep_set(set);

    GRBColumn col;
    for (const node& n : set) {
        HANDLE_GRB_EXCEPTION(col.addTerm(1.0, constrs[n]));
    }

    HANDLE_GRB_EXCEPTION(
        vars[set] = grb.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, col));
    grb.update();
}

color_sol solver::solve()
{
    LOG_SCOPE_F(INFO, "Solver.");

    const graph& g = form.get_graph();
    while (true) {
        HANDLE_GRB_EXCEPTION(grb.optimize());

        HANDLE_GRB_EXCEPTION(grb.get(GRB_DoubleAttr_Runtime));
        HANDLE_GRB_EXCEPTION(grb.get(GRB_DoubleAttr_ObjVal));
        LOG_F(INFO,
              "Solved in %lf with value %lf",
              grb.get(GRB_DoubleAttr_Runtime),
              grb.get(GRB_DoubleAttr_ObjVal));

        // get the weights of the nodes from the dual variables
        for_nodes(g, v) {
            form.set_weight(v, constrs[v].get(GRB_DoubleAttr_Pi));
        }

        vector<node_set> const sets = pricing::solve(g);

        if (sets.empty()) {
            LOG_F(INFO, "No more sets to add.");
            break;
        }

        for (const node_set& set : sets) {
            add_variable(set);
        }
    }

    LOG_F(INFO, "Final model with %d sets.", (int)vars.size());

    color_sol sol;
    for_indep_set(form, set) {
        sol.x_sets[set] = vars[set].get(GRB_DoubleAttr_X);
    }
    sol.cost = 0;
    for_nodes(g, u) {
        sol.cost += EPS * floor(g.get_weight(u) / EPS);
    }
    // TODO no caso de variáveis de corte, isso deve ser ceil
    // conferir o paper do Lotti
    return sol;
}
