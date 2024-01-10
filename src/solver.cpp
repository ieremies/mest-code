#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

#include "../incl/solver.hpp"

#include <gurobi_c++.h>

#include "../incl/pricing.hpp"
#include "../incl/utils.hpp"

Solver::Solver()
    : _env(true)
{
    // disable gurobi license output
    _env.set(GRB_DoubleParam_TimeLimit, TIMELIMIT);
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
void write_mwis(const Graph& g,
                const vector<double>& weight,
                const string& filename)
{
    ofstream file(filename);
    file << g.get_n() << " " << g.get_m() << endl;
    for_nodes(g, n) {
        // transform weight to integer
        int w = (int)round(weight[n] * 214748);
        file << w << " ";
        for_adj(g, n, u) {
            file << u + 1 << " ";
        }
        file << endl;
    }
    file.close();
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
    for (const node& n : set) {
        c += var[n];
    }
    constrs[set] = model.addConstr(c <= 1.0);

    indep_sets.push_back(set);
}

double Solver::solve(const Graph& g,
                     vector<node_set>& indep_sets,
                     map<node_set, double>& x_s)
{
    LOG_SCOPE_F(INFO, "Solver.");

    GRBModel model(_env);
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // each vertex has a weight
    vector<GRBVar> vars(g.get_n());
    for_nodes(g, n) {
        vars[n] = model.addVar(
            0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, "node " + to_string(n));
    }

    model.update();

    // BUG atualmente, existem momentos no qual, ao desfazer contracts, os nós
    // que retornam podem não serem cobertos por nenhum conjunto independente.
    // Isso faz com que o modelo seja inviável
    vector<bool> is_used(g.get_n(), false);

    // sum of weights of nodes in an independent set <= 1
    map<node_set, GRBConstr> constrs;
    for (const node_set& set : indep_sets) {
        DCHECK_F(!set.empty(), "Empty set in the list.");

        GRBLinExpr c = 0;
        for (node n : set) {
            DCHECK_F(g.is_active(n), "node is not active.");
            c += vars[n];
            is_used[n] = true;
        }

        constrs[set] = model.addConstr(c <= 1.0);
    }

    for_nodes(g, u) {
        if (!is_used[u]) {
            add_constrain(model, vars, constrs, indep_sets, {u});
        }
    }

    LOG_F(INFO, "Initial model with %d sets.", (int)indep_sets.size());

    vector<double> weight(g.get_n());
    int i = 0;
    while (true) {
        i++;
        model.optimize();
        LOG_F(INFO,
              "Solved in %lf with value %lf",
              model.get(GRB_DoubleAttr_Runtime),
              model.get(GRB_DoubleAttr_ObjVal));

        // get the weights of the nodes
        for_nodes(g, n) {
            weight[n] = vars[n].get(GRB_DoubleAttr_X);
        }

        write_mwis(g, weight, "mwis_" + to_string(i));
        vector<node_set> sets = pricing::solve(g, weight);

        if (sets.empty()) {
            break;
        }

        for (const node_set& set : sets) {
            add_constrain(model, vars, constrs, indep_sets, set);
        }
    }

    LOG_F(INFO, "Final model with %d sets.", (int)indep_sets.size());

    // for each constrain, get its shadow price and save
    // it as the correspondent x_s
    for (const node_set& set : indep_sets) {
        x_s[set] = EPS * floor((constrs[set].get(GRB_DoubleAttr_Pi) / EPS));
    }

    // Return the dual objective solution
    return model.get(GRB_DoubleAttr_ObjVal);
}
