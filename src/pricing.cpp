#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"
#include <gurobi_c++.h>

// maximize     x_v weight_v
// subjected to x_v + x_w <= 1 for all edges (v,w)
//              x_v binary for all nodes v
NodeSet Pricing::solve(const Graph &g, const Graph::NodeMap<double> &weight) {
    LOG_SCOPE_F(INFO, "Pricing.");

    GRBEnv env = GRBEnv();
    env.set(GRB_IntParam_LogToConsole, 0);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.set(GRB_IntParam_Threads, 1);
    env.start();

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

    // If there is no set with weight above 1, we are done.
    if (pricing_model.get(GRB_DoubleAttr_ObjVal) <= 1) {
        LOG_F(INFO, "No set with weight above 1. (cost %f)",
              pricing_model.get(GRB_DoubleAttr_ObjVal));
        return {};
    }
    // Retrieve the set of selected nodes.
    NodeSet set;
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n)
        if (x[n].get(GRB_DoubleAttr_X) > 0.5)
            set.insert(n);

    LOG_F(INFO, "Princing with cost %f, set %s.",
          pricing_model.get(GRB_DoubleAttr_ObjVal),
          Utils::NodeSet_to_string(g, set).c_str());
    return set;
}
