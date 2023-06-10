#include <algorithm>

#include "../incl/heuristic.hpp"

#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"

cost dsatur(const Graph& graph, vector<node_set>& indep_sets)
{
    LOG_SCOPE_FUNCTION(INFO);
    // neighbors_color[n] is a vector of bools, where neighbors_color[n][i] is
    // true if node n has a neighbor with color i.
    vector<vector<bool>> neighbors_colors(graph.get_n(),
                                          vector<bool>(graph.get_n(), false));

    // sat_deg[n] = sum(neighbors_colors[n]);
    vector<int> sat_deg(graph.get_n(), 0);

    // Zero means not colored
    vector<node> vertex_color(graph.get_n(), 0);

    node atual = max_element(sat_deg.begin(), sat_deg.end()) - sat_deg.begin();
    DLOG_F(INFO, "DSATUR: first node %d ", atual);

    // Log sat_deg vector

    do {
        // color it with the lowest color label available for it.
        color to_use = graph.get_n();
        for (color i = 1; i < graph.get_n(); i++) {
            if (!neighbors_colors[atual][i]) {
                to_use = i;
                break;
            }
        }
        vertex_color[atual] = to_use;
        sat_deg[atual] = -1;

        // For each adjacent node of "atual" that has not been colored yet,
        // add "to_use" to its list of used colors.
        for_adj(graph, atual, n) {
            if (vertex_color[n] == 0 and !neighbors_colors[n][to_use]) {
                neighbors_colors[n][to_use] = true;
                sat_deg[n]++;
            }
        }
        atual = max_element(sat_deg.begin(), sat_deg.end()) - sat_deg.begin();
        DLOG_F(INFO, "DSATUR: next node %d ", atual);
    } while (sat_deg[atual] > 0 or vertex_color[atual] == 0);

    color res = *max_element(vertex_color.begin(), vertex_color.end());
    DLOG_F(INFO, "DSATUR: %d colors", res);

    // Create the independent sets
    indep_sets.resize(res);
    for (node n = 0; n < graph.get_n(); n++) {
        indep_sets[vertex_color[n] - 1].insert(n);
    }

    return res;
}
