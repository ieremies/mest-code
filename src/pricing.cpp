#include "../incl/pricing.hpp"
#include <gurobi_c++.h>

// maximize     x_v weight_v
// subjected to x_v + x_w <= 1 for all edges (v,w)
//              x_v binary for all nodes v
int Pricing::solve(const Graph &g, const Graph::NodeMap<double> &weight,
                   std::vector<NodeSet> &indep_sets) {
    GRBEnv env = GRBEnv();
    GRBModel model(env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // x_v is 1 if v is selected, 0 otherwise.
    Graph::NodeMap<GRBVar> x(g);
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n) {
        x[n] = model.addVar(0.0, 1.0, weight[n], GRB_BINARY,
                            "x_" + std::to_string(g.id(n)));
    }

    model.update();

    // No adjacent nodes in g are both selected.
    for (Graph::EdgeIt e(g); e != lemon::INVALID; ++e) {
        Graph::Node n1 = g.u(e);
        Graph::Node n2 = g.v(e);
        model.addConstr(x[n1] + x[n2] <= 1, "edge " + std::to_string(g.id(n1)) +
                                                " " + std::to_string(g.id(n2)));
    }

    model.optimize();

    // BUG esse set pode deixar de existir??
    NodeSet set;
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        if (x[n].get(GRB_DoubleAttr_X) > 0.5)
            set.insert(n);
    if (set.size() > 0) {
        indep_sets.push_back(set);
        return 1;
    }
    return 0;
}
