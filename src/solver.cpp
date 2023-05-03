#include "../incl/solver.hpp"
#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"
#include <gurobi_c++.h>
#include <gurobi_c.h>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <set>
#include <string>
#include <vector>

Solver::Solver() : _env(GRBEnv()) {
    _env.set(GRB_IntParam_Threads, 1);
    _env.set(GRB_DoubleParam_TimeLimit, TIMELIMIT);
    _env.set(GRB_DoubleParam_FeasibilityTol, 1e-9);
    _env.set(GRB_DoubleParam_OptimalityTol, 1e-9);
    _env.set(GRB_IntParam_NumericFocus, 1);
    _env.set(GRB_IntParam_OutputFlag, 0);
    _env.start();
}

Solver::~Solver() {}

void add_constrain(GRBModel &model, const Graph::NodeMap<GRBVar> &var,
                   std::map<NodeSet, GRBConstr> &constrs,
                   std::vector<NodeSet> &indep_sets, const NodeSet &set) {

    CHECK_F(std::find(indep_sets.begin(), indep_sets.end(), set) ==
                indep_sets.end(),
            "The set is already in the list.");

    GRBLinExpr c = 0;
    for (Graph::Node node : set)
        c += var[node];
    constrs[set] = model.addConstr(c <= 1.0);

    indep_sets.push_back(set);
    LOG_F(INFO, "We have now %d independent sets.", (int)indep_sets.size());
}

double Solver::solve(const Graph &g, std::vector<NodeSet> &indep_sets,
                     std::map<NodeSet, double> &x_s) {
    LOG_SCOPE_F(INFO, "Solver.");
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // each vertex has a weight
    Graph::NodeMap<GRBVar> var(g);
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        var[n] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS,
                              "node " + std::to_string(g.id(n)));

    model.update();

    // sum of weights of nodes in an independent set <= 1
    std::map<NodeSet, GRBConstr> constrs;
    for (NodeSet set : indep_sets) {
        CHECK_F(set.size() > 0, "Empty set in the list.");
        GRBLinExpr c = 0;
        for (Graph::Node node : set)
            c += var[node];
        constrs[set] = model.addConstr(c <= 1.0);
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)indep_sets.size());
    Graph::NodeMap<double> weight(g);
    while (true) {
        model.optimize();

        // get the weights of the nodes
        for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
            weight[n] = var[n].get(GRB_DoubleAttr_X);

        NodeSet set = Pricing::solve(g, weight);

        if (set.empty()) {
            LOG_F(INFO, "New set is empty. Stopping.");
            break;
        }

        add_constrain(model, var, constrs, indep_sets, set);
    }

    // for each constrain, get its shadow price and save
    // it as the correspondent x_s
    for (NodeSet set : indep_sets) {
        x_s[set] = constrs[set].get(GRB_DoubleAttr_Pi);
    }

    // Return the dual objective solution
    return model.get(GRB_DoubleAttr_ObjVal);
}
