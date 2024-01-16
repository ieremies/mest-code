#include <limits>

#include "../incl/pricing.hpp"

extern "C"
{
#include "../lib/cliquer-1.21/cliquer.h"
}

enum
{
    max_clique = 100
};

vector<node_set> pricing::solve(const Graph& g, const vector<double>& weight)
{
    LOG_SCOPE_F(INFO, "Pricing.");

    vector<pair<node, double>> non_zero;
    map<node, int> trans;
    map<int, node> undo;

    int qtd = 0;
    for (auto w : weight) {
        if (w > EPS) {
            qtd++;
        }
    }
    // max int divided by the number of nodes
    const int precision = numeric_limits<int>::max() / qtd;

    for_nodes(g, u) {
        if ((int)(weight[u] * precision) > 0) {
            trans[u] = (int)non_zero.size();
            undo[(int)non_zero.size()] = u;
            non_zero.push_back({u, weight[u]});
        }
    }

    graph_t* g_cliquer = graph_new(non_zero.size());
    for (auto [u, w] : non_zero) {
        g_cliquer->weights[trans[u]] = (int)(w * precision);
        // LOG_F(INFO, "Node %d has weight %d", u,
        // g_cliquer->weights[trans[u]]);

        for_nodes(g, v) {
            if ((int)(weight[v] * precision) > 0 and g.get_adjacency(u, v) == 0
                and u < v)
            {
                GRAPH_ADD_EDGE(g_cliquer, trans[u], trans[v]);
            }
        }
    }
    DCHECK_F(graph_test(g_cliquer, NULL), "Graph is not valid.");

    set_t clique_list[max_clique];
    clique_default_options->clique_list = clique_list;
    clique_default_options->clique_list_length = max_clique;
    clique_default_options->time_function = NULL;
    qtd =
        clique_find_all(g_cliquer, precision + 1, 0, 1, clique_default_options);

    LOG_F(INFO, "Found %d cliques.", qtd);

    vector<node_set> indep_sets = {};
    for (int i = 0; i < qtd and i < max_clique; i++) {
        node_set set = {};
        for (setelement j = 0; j < non_zero.size(); j++) {
            if (SET_CONTAINS(clique_list[i], j)) {
                set.insert(undo[j]);
            }
        }
        // TODO Write a function to make a set maximal that checks if node is
        // active
        double sum = 0;
        for (node n : set) {
            sum += weight[n];
        }
        if (sum > 1 - EPS) {
            indep_sets.push_back(set);
        }
    }

    // log indep_sets
    for (auto set : indep_sets) {
        LOG_F(INFO, "Found set %s.", to_string(set).c_str());
    }

    return indep_sets;
}
