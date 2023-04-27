#include "../incl/solver.hpp"
#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"
#include <gurobi_c++.h>
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
    _env.start();
}

Solver::~Solver() {}

double Solver::solve(const Graph &g, std::vector<NodeSet> &indep_sets,
                     std::vector<double> &x_s) {
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // each vertex has a weight
    Graph::NodeMap<GRBVar> var(g);
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        var[n] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS,
                              "node " + std::to_string(g.id(n)));

    model.update();

    // sum of weights of nodes in an independent set <= 1
    std::vector<GRBConstr> constrs;
    for (NodeSet set : indep_sets) {
        if (set.empty())
            continue;
        GRBLinExpr c = 0;
        for (Graph::Node node : set)
            c += var[node];
        GRBConstr constr = model.addConstr(c <= 1.0);
        constrs.push_back(constr);
    }

    Graph::NodeMap<double> weight(g);
    do {
        model.optimize();

        // get the weights of the nodes
        for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
            weight[n] = var[n].get(GRB_DoubleAttr_X);

    } while (Pricing::solve(g, weight, indep_sets, model, var, constrs));

    // for each constrain, get its shadow price and save
    // it as the correspondent x_s
    x_s.resize(constrs.size());
    for (int i = 0; i < constrs.size(); i++)
        x_s[i] = constrs[i].get(GRB_DoubleAttr_Pi);

    // Return solution
    return model.get(GRB_DoubleAttr_ObjVal);
}
