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
** Add a new constrain to the model and update the list of independent sets.
** The constrain is that the sum of the weights of the nodes in the set is <= 1.
*/
void add_constrain(GRBModel& model,
                   const vector<GRBVar>& var,
                   map<node_set, GRBConstr>& constrs,
                   vector<node_set>& indep_sets,
                   const node_set& set)
{
    DCHECK_F(
        find(indep_sets.begin(), indep_sets.end(), set) == indep_sets.end(),
        "The set %s is already in the list.",
        to_string(set).c_str());

    GRBLinExpr c = 0;
    for (const node& n : set) {
        c += var[n];
    }

    HANDLE_GRB_EXCEPTION(constrs[set] = model.addConstr(c <= 1.0));

    indep_sets.push_back(set);
}

cost Solver::solve(Graph& g,
                   vector<node_set>& indep_sets,
                   map<node_set, cost>& x_s)
{
    LOG_SCOPE_F(INFO, "Solver.");
    DCHECK_F(g.get_n() > 0, "Graph is empty.");
    DCHECK_F(check_indep_sets(g, indep_sets), "Invalid independent sets.");

    // === Create the model ===
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    // change method to dual simplex
    // TODO check if its faster
    model.set(GRB_IntParam_Method, 1);

    // each vertex has a weight
    vector<GRBVar> vars(g.get_n());
    for_nodes(g, n) {
        vars[n] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS);
    }

    model.update();

    // sum of weights of nodes in an independent set <= 1
    map<node_set, GRBConstr> constrs;
    for (const node_set& set : indep_sets) {
        DCHECK_F(!set.empty(), "Empty set in the list.");

        GRBLinExpr c = 0;
        for (node const n : set) {
            c += vars[n];
        }

        HANDLE_GRB_EXCEPTION(constrs[set] = model.addConstr(c <= 1.0));
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)indep_sets.size());

    // === Solve the model ===
    while (true) {
        HANDLE_GRB_EXCEPTION(model.optimize());

        LOG_F(INFO,
              "Solved in %lf with value %lf",
              model.get(GRB_DoubleAttr_Runtime),
              model.get(GRB_DoubleAttr_ObjVal));

        // get the weights of the nodes
        for_nodes(g, n) {
            g.set_weight(n, vars[n].get(GRB_DoubleAttr_Pi));
        }

        vector<node_set> const sets = pricing::solve(g);

        if (sets.empty()) {
            LOG_F(INFO, "No more sets to add.");
            break;
        }

        DCHECK_F(check_indep_sets(g, sets), "Invalid new sets.");

        for (const node_set& set : sets) {
            add_constrain(model, vars, constrs, indep_sets, set);
        }
        LOG_F(INFO, "Added %d sets.", (int)sets.size());
    }

    LOG_F(INFO, "Final model with %d sets.", (int)indep_sets.size());

    // for each constrain, get its shadow price and save
    // it as the correspondent x_s
    for (const node_set& set : indep_sets) {
        x_s[set] = constrs[set].get(GRB_DoubleAttr_Pi);
    }

    // Return the dual objective solution
    cost sum = 0;
    for_nodes(g, u) {
        sum += EPS * floor(g.get_weight(u) / EPS);
    }
    return sum;
}
