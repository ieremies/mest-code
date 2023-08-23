#include "../incl/pricing.hpp"

#include <gurobi_c++.h>

#include "../incl/utils.hpp"

// maximize     x_v weight_v
// subjected to x_v + x_w <= 1 for all edges (v,w)
//              x_v binary for all nodes v
vector<node_set> pricing::solve(const Graph& g, const vector<double>& weight)
{
    LOG_SCOPE_F(INFO, "Pricing.");

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_LogToConsole, 0);
    env.set(GRB_IntParam_OutputFlag, 0);
    if (SINGLE_THREAD) {
        env.set(GRB_IntParam_Threads, 1);
    }
    env.start();

    double lower_bound = 0;
    for_nodes(g, n) {
        double dom = weight[n];
        for_adj(g, n, u) {
            if (n != u) {
                dom += weight[u];
            }
        }
        lower_bound = weight[n] * weight[n] / dom;
    }
    LOG_F(INFO, "Lower bound: %f", lower_bound);

    // Maximize model.
    GRBModel pricing_model(env);
    pricing_model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    // TODO Get multiple solutions.
    // pricing_model.set(GRB_IntParam_SolutionLimit, 16 * g.get_n());
    // pricing_model.set(GRB_IntParam_PoolSearchMode, 0);

    // x_v is 1 if v is selected, 0 otherwise.
    vector<GRBVar> x(g.get_n());
    for_nodes(g, n) {
        x[n] = pricing_model.addVar(
            0.0, 1.0, weight[n], GRB_BINARY, "x_" + to_string(n));
    }

    pricing_model.update();

    // No adjacent nodes in g are both selected.
    for_nodes(g, n1) {
        for_nodes(g, n2) {
            if (g.get_incidency(n1, n2) > 0) {
                pricing_model.addConstr(
                    x[n1] + x[n2] <= 1,
                    "edge " + to_string(n1) + " " + to_string(n2));
            }
        }
    }

    pricing_model.optimize();

    vector<node_set> sets = {};

    // While we can find a set with weight > 1.
    while (pricing_model.get(GRB_DoubleAttr_ObjVal) >= 1 + EPS) {
        // Retrive the found set.
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

        sets.push_back(set);

        // element with max weight in the set chosen
        double max_weight = -1;
        int max_weight_node = -1;
        for (node n : set) {
            if (x[n].get(GRB_DoubleAttr_X) > 0.5 && weight[n] > max_weight) {
                max_weight = weight[n];
                max_weight_node = n;
            }
        }
        // Remove it and solve again.
        pricing_model.addConstr(x[max_weight_node] == 0,
                                "remove " + to_string(max_weight_node));
        pricing_model.optimize();
    }

    LOG_F(INFO, "%lu new sets.", sets.size());

    return sets;
}
