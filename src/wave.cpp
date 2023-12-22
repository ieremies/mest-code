#include <algorithm>
#include <vector>

#include "graph.hpp"
#include "heuristic.hpp"
#include "utils.hpp"

#define MAX_ENTROPY 1000000
// TODO do by bitmap
cost heuristic(const Graph& g, vector<node_set>& indep_sets)
{
    LOG_SCOPE_FUNCTION(INFO);

    // find the vertex with max degree
    node u = g.first_act_node();
    for_nodes(g, v) {
        if (g.get_degree(v) > g.get_degree(u)) {
            u = v;
        }
    }

    // Max number of colors might be wrong, but it's a good upper bound
    int const max_colors = g.get_n();

    // vector of colors (0 = uncolored)
    vector<color> colors(max_colors, 0);
    int qtd_colored = 0;

    // vector of entropy (number of available colors)
    vector<int> entro(g.get_n(), max_colors);

    // matrix of possible colors
    // possible[i][j] is true if color j is possible for node i
    vector<vector<bool>> possible(g.get_n(), vector<bool>(max_colors, true));

    // color the vertex with max degree
    colors[u] = 1;
    qtd_colored++;
    entro[u] = MAX_ENTROPY;
    // collapse
    for_adj(g, u, n) {
        if (possible[n][colors[u]]) {
            possible[n][colors[u]] = false;
            entro[n]--;
        }
    }

    // while 0 in c
    while (qtd_colored < g.get_n()) {
        // find the min element in e
        u = min_element(entro.begin(), entro.end()) - entro.begin();

        // min available color for min_e
        auto min_c = 1;
        while (!possible[u][min_c]) {
            min_c++;
        }

        colors[u] = min_c;
        qtd_colored++;
        entro[u] = MAX_ENTROPY;
        // collapse
        for_adj(g, u, n) {
            if (possible[n][colors[u]]) {
                possible[n][colors[u]] = false;
                entro[n]--;
            }
        }
    }

    color const res = *max_element(colors.begin(), colors.end());
    DLOG_F(INFO, "WAVE: %d colors", res);

    indep_sets.resize(res);
    for_nodes(g, v) {
        indep_sets[colors[v] - 1].insert(v);
    }

    string log = "SOL: %f = ";
    for (node_set const& s : indep_sets) {
        log += to_string(s) + " ";
    }
    LOG_F(WARNING, log.c_str(), (double)indep_sets.size());

    return res;
}
