#include "../incl/pricing.hpp"
#include <gurobi_c++.h>

// maximize     x_v weight_v
// subjected to x_v + x_w <= 1 for all edges (v,w)
//              x_v binary for all nodes v
bool Pricing::solve(const Graph &g, const Graph::NodeMap<double> &weight,
                    std::vector<NodeSet> &indep_sets, GRBModel &model,
                    const Graph::NodeMap<GRBVar> &var,
                    std::vector<GRBConstr> &constrs) {
    GRBEnv env = GRBEnv();
    GRBModel pricing_model(env);
    pricing_model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // x_v is 1 if v is selected, 0 otherwise.
    Graph::NodeMap<GRBVar> x(g);
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n) {
        x[n] = pricing_model.addVar(0.0, 1.0, weight[n], GRB_BINARY,
                                    "x_" + std::to_string(g.id(n)));
    }

    pricing_model.update();

    // No adjacent nodes in g are both selected.
    for (Graph::EdgeIt e(g); e != lemon::INVALID; ++e) {
        Graph::Node n1 = g.u(e);
        Graph::Node n2 = g.v(e);
        pricing_model.addConstr(x[n1] + x[n2] <= 1,
                                "edge " + std::to_string(g.id(n1)) + " " +
                                    std::to_string(g.id(n2)));
    }

    pricing_model.optimize();

    // Retrieve the set of selected nodes.
    NodeSet set;
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        if (x[n].get(GRB_DoubleAttr_X) > 0.5)
            set.insert(n);

    // If there is no set with weight above 1, we are done.
    if (set.size() == 0)
        return false;

    // If the chosen set is already in the list, we are done.
    for (NodeSet s : indep_sets)
        if (s == set)
            return false;

    // Otherwise, add the set to the list.
    indep_sets.push_back(set);
    GRBLinExpr c;
    for (auto node : set)
        c += var[node];
    constrs.push_back(model.addConstr(c <= 1));

    return true;
}
