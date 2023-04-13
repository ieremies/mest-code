#include "../incl/solver.hpp"
#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"
#include <gurobi_c++.h>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <set>
#include <string>
#include <vector>

// Isso aqui vai me dar BO já que eu tenho que colocar uns parâmetros.
GRBEnv Solver::_env = GRBEnv();

void Solver::solve(const Graph &g, const std::vector<NodeSet> &indep_sets,
                   Graph::NodeMap<double> &weight) {
    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // each vertex has a weight
    Graph::NodeMap<GRBVar> var(g);
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        var[n] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS,
                              "node " + std::to_string(g.id(n)));

    model.update();

    // sum of weights of nodes in an independent set <= 1
    for (auto set : indep_sets) {
        GRBLinExpr c;
        for (auto node : set)
            c += var[node];
        model.addConstr(c <= 1);
    }

    model.optimize();

    // get the weights of the nodes
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        weight[n] = var[n].get(GRB_DoubleAttr_X);
}
