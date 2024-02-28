#include <algorithm>
#include <cmath>
#include <vector>

#include "../incl/solver.hpp"

#include <gurobi_c++.h>

#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"

Solver::Solver()
    : _env(true)
{
    // disable gurobi license output
    _env.set(GRB_DoubleParam_FeasibilityTol, EPS);
    _env.set(GRB_DoubleParam_OptimalityTol, EPS);
    _env.set(GRB_IntParam_LogToConsole, 0);
    _env.set(GRB_IntParam_OutputFlag, 0);
    _env.set(GRB_IntParam_NumericFocus, 1);
    // make gurobi use only one thread
    if (SINGLE_THREAD) {
        _env.set(GRB_IntParam_Threads, 1);
    }
    _env.start();
}

/*
** Add a new variable to the model and update the list of independent sets.
** We also need to compute in which constraints this variable will be used.
*/
void add_variable(GRBModel& model,
                  map<node_set, GRBVar>& vars,
                  vector<GRBConstr>& constrs,
                  vector<node_set>& indep_sets,
                  const node_set& set)
{
    if (vars.find(set) != vars.end()) {
        LOG_F(ERROR, "Skipping set already exists.");
        return;
    }

    GRBColumn col;
    for (const node& n : set) {
        HANDLE_GRB_EXCEPTION(col.addTerm(1.0, constrs[n]));
    }

    HANDLE_GRB_EXCEPTION(
        vars[set] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, col));
    model.update();

    indep_sets.push_back(set);
}

cost Solver::solve(const Graph& g,
                   vector<node_set>& indep_sets,
                   map<node_set, cost>& x_s)
{
    LOG_SCOPE_F(INFO, "Solver.");
    DCHECK_F(g.get_n() > 0, "Graph is empty.");
    DCHECK_F(check_indep_sets(g, indep_sets), "Invalid independent sets.");

    // === Create the model ===
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    // disable presolve
    model.set(GRB_IntParam_Presolve, 0);
    model.set(GRB_IntParam_Method, 0);

    // Each independent set has a variable
    map<node_set, GRBVar> vars;
    for (const auto& set : indep_sets) {
        vars[set] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS);
    }

    model.update();

    // For each vertice, it has to covered by at least one set
    vector<GRBConstr> constrs = vector<GRBConstr>(g.get_n());
    for_nodes(g, v) {
        GRBLinExpr c = 0;
        for (const auto& set : indep_sets) {
            if (set.count(v) > 0) {
                c += vars[set];
            }
        }

        HANDLE_GRB_EXCEPTION(constrs[v] = model.addConstr(c >= 1.0));
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)indep_sets.size());

    // === Solve the model ===
    vector<cost> weight(g.get_n());
    while (true) {
        HANDLE_GRB_EXCEPTION(model.optimize());

        LOG_F(INFO,
              "Solved in %lf with value %lf",
              model.get(GRB_DoubleAttr_Runtime),
              model.get(GRB_DoubleAttr_ObjVal));

        // get the weights of the nodes from the dual variables
        for_nodes(g, v) {
            weight[v] = constrs[v].get(GRB_DoubleAttr_Pi);
        }

        vector<node_set> const sets = pricing::solve(g, weight);

        if (sets.empty()) {
            LOG_F(INFO, "No more sets to add.");
            break;
        }

        DCHECK_F(check_indep_sets(g, sets), "Invalid new sets.");

        for (const node_set& set : sets) {
            add_variable(model, vars, constrs, indep_sets, set);
        }
    }

    LOG_F(INFO, "Final model with %d sets.", (int)indep_sets.size());

    for (const node_set& set : indep_sets) {
        x_s[set] = vars[set].get(GRB_DoubleAttr_X);
    }

    // Return the dual objective solution
    cost sum = 0;
    for_nodes(g, u) {
        sum += EPS * floor(weight[u] / EPS);
    }
    // TODO no caso de variáveis de corte, isso deve ser ceil
    // conferir o paper do Lotti
    return sum;
}
