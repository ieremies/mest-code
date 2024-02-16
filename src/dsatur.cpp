#include <algorithm>

#include "../incl/heuristic.hpp"
#include "../incl/utils.hpp"

cost heuristic(const Graph& graph, vector<node_set>& indep_sets)
{
    LOG_SCOPE_FUNCTION(INFO);

    // neighbors_color[n] is a vector of bools, where neighbors_color[n][i] is
    // true if node n has a neighbor with color i.
    vector<vector<bool>> neighbors_colors(graph.get_n(),
                                          vector<bool>(graph.get_n(), false));

    // vector<int> sat_deg(graph.get_n(), 0);
    // sat_deg is a vector pair (saturation degree, vertex degree)
    vector<pair<int, int>> sat_deg(graph.get_n(), {0, 0});
    for_nodes(graph, u) {
        sat_deg[u].second = graph.get_degree(u);
    }
    vector<node> vertex_color(graph.get_n(), 0);  // Zero means not colored

    node atual = 0;

    while (true) {
        // Find the node with the highest saturation degree, breaking ties by
        // the highest degree.
        atual = std::max_element(
                    sat_deg.begin(),
                    sat_deg.end(),
                    [](const pair<int, int>& a, const pair<int, int>& b)
                    {
                        if (a.first == b.first) {
                            return a.second < b.second;
                        }
                        return a.first < b.first;
                    })
            - sat_deg.begin();

        if (sat_deg[atual].first < 0) {
            break;
        }

        // color it with the lowest color label available for it.
        color to_use = graph.get_n();
        for (color i = 1; i < graph.get_n(); i++) {
            if (!neighbors_colors[atual][i]) {
                to_use = i;
                break;
            }
        }
        vertex_color[atual] = to_use;
        sat_deg[atual].first = -1;

        // For each adjacent node of "atual" that has not been colored yet,
        // add "to_use" to its list of used colors.
        for_adj(graph, atual, n) {
            if (vertex_color[n] == 0 and !neighbors_colors[n][to_use]) {
                neighbors_colors[n][to_use] = true;
                sat_deg[n].first++;
            }
        }
    }

    color const res = *max_element(vertex_color.begin(), vertex_color.end());
    DLOG_F(INFO, "DSATUR: %d colors", res);

    // Create the independent sets
    indep_sets.resize(res);
    for (node n = 0; n < graph.get_n(); n++) {
        indep_sets[vertex_color[n] - 1].insert(n);
    }

    string log = "SOL: %f = ";
    for (node_set const& s : indep_sets) {
        log += to_string(s) + " ";
    }
    LOG_F(WARNING, log.c_str(), (double)indep_sets.size());

    return res;
}
