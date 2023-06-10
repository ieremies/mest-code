#include "../incl/pricing.hpp"

#include <gurobi_c++.h>

#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"

// maximize     x_v weight_v
// subjected to x_v + x_w <= 1 for all edges (v,w)
//              x_v binary for all nodes v
node_set Pricing::solve(const Graph& g, const vector<double>& weight)
{
    LOG_SCOPE_F(1, "Pricing.");

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_LogToConsole, 0);
    env.set(GRB_IntParam_OutputFlag, 0);
    if (SINGLE_THREAD) {
        env.set(GRB_IntParam_Threads, 1);
    }
    env.start();

    GRBModel pricing_model(env);
    pricing_model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // x_v is 1 if v is selected, 0 otherwise.
    vector<GRBVar> x(g.get_n());
    for_nodes(g, n) {
        x[n] = pricing_model.addVar(
            0.0, 1.0, weight[n], GRB_BINARY, "x_" + to_string(n));
    }
    pricing_model.update();

    // No adjacent nodes in g are both selected.
    // BUG not performant
    for_nodes(g, n1)
        for_nodes(g, n2)
            if (g.get_incidency(n1, n2) > 0) {
                pricing_model.addConstr(
                    x[n1] + x[n2] <= 1,
                    "edge " + to_string(n1) + " " + to_string(n2));
            }

    pricing_model.optimize();

    // If there is no set with weight above 1, we are done.
    if (pricing_model.get(GRB_DoubleAttr_ObjVal) <= 1 + EPS) {
        LOG_F(1,
              "No set with weight above 1. (cost %f)",
              pricing_model.get(GRB_DoubleAttr_ObjVal));
        return {};
    }
    /*
    ** TODO para pegar mais de um conjunto, uma forma útil é remover os vértices
    ** selecionados desse ILP e rodar de novo.
    */
    // Retrieve the set of selected nodes.
    node_set set;
    for_nodes(g, n) {
        if (x[n].get(GRB_DoubleAttr_X) > 0.5) {
            set.insert(n);
        }
    }

    LOG_F(1,
          "Princing with cost %f, set %s.",
          pricing_model.get(GRB_DoubleAttr_ObjVal),
          to_string(set).c_str());
    return set;
}
