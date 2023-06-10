#include <algorithm>
#include <vector>

#include "../incl/solver.hpp"

#include <gurobi_c++.h>

#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"

Solver::Solver()
    : _env(true)
{
    // disable gurobi license output
    _env.set(GRB_IntParam_LogToConsole, 0);
    _env.set(GRB_DoubleParam_TimeLimit, TIMELIMIT);
    _env.set(GRB_DoubleParam_FeasibilityTol, EPS);
    _env.set(GRB_DoubleParam_OptimalityTol, EPS);
    _env.set(GRB_IntParam_OutputFlag, 0);
    _env.set(GRB_IntParam_NumericFocus, 1);
    // make gurobi use only one thread
    if (SINGLE_THREAD) {
        _env.set(GRB_IntParam_Threads, 1);
    }
    _env.start();
}

void add_constrain(GRBModel& model,
                   const vector<GRBVar>& var,
                   map<node_set, GRBConstr>& constrs,
                   vector<node_set>& indep_sets,
                   const node_set& set)
{
    DCHECK_F(
        find(indep_sets.begin(), indep_sets.end(), set) == indep_sets.end(),
        "The set is already in the list.");

    GRBLinExpr c = 0;
    for (node n : set)
        c += var[n];
    constrs[set] = model.addConstr(c <= 1.0);

    indep_sets.push_back(set);
    LOG_F(1, "We have now %d independent sets.", (int)indep_sets.size());
}

double Solver::solve(const Graph& g,
                     vector<node_set>& indep_sets,
                     map<node_set, double>& x_s)
{
    LOG_SCOPE_F(INFO, "Solver.");
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // each vertex has a weight
    // BUG cuidado, haverá vars não incializadas
    vector<GRBVar> vars(g.get_n());
    for_nodes(g, n) {
        vars[n] = model.addVar(
            0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, "node " + to_string(n));
    }

    model.update();

    // sum of weights of nodes in an independent set <= 1
    map<node_set, GRBConstr> constrs;
    for (const node_set& set : indep_sets) {
        CHECK_F(!set.empty(), "Empty set in the list.");
        GRBLinExpr c = 0;
        for (node n : set) {
            CHECK_F(g.is_active(n), "node is not active.");
            c += vars[n];
        }
        constrs[set] = model.addConstr(c <= 1.0);
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)indep_sets.size());
    // for (const node_set& set : indep_sets) {
    //     LOG_F(INFO, "Set %s", to_string(set).c_str());
    // }

    vector<double> weight(g.get_n());
    while (true) {
        model.optimize();

        // get the weights of the nodes
        for_nodes(g, n) {
            weight[n] = vars[n].get(GRB_DoubleAttr_X);
        }

        node_set set = Pricing::solve(g, weight);

        if (set.empty()) {
            LOG_F(INFO,
                  "New set is empty. Stopping with %ld sets.",
                  indep_sets.size());
            break;
        }

        add_constrain(model, vars, constrs, indep_sets, set);
    }

    // for each constrain, get its shadow price and save
    // it as the correspondent x_s
    for (const node_set& set : indep_sets) {
        x_s[set] = constrs[set].get(GRB_DoubleAttr_Pi);
    }

    // Return the dual objective solution
    return model.get(GRB_DoubleAttr_ObjVal);
}
