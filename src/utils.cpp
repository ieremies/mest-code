#include "../incl/utils.hpp"

string to_string(const node_set& set)
{
    string s = "{";
    for (node const u : set) {
        if (s.size() > 1) {
            s += ", ";
        }
        s += to_string(u);
    }
    s += "}";
    return s;
}

void log_solution(const Graph& g,
                  const map<node_set, double>& x_s,
                  const cost& sol)
{
    vector<node_set> indep_sets;
    for (const auto& [s, x] : x_s) {
        if (x >= 1 - EPS) {
            indep_sets.push_back(s);
        }
    }
    g.apply_changes_to_sol(indep_sets);
    string log = "SOL: %f = ";
    for (const auto& s : indep_sets) {
        log += to_string(s) + " ";
    }
    LOG_F(WARNING, log.c_str(), sol);
}

bool integral(const map<node_set, double>& x_s)
{
    for (const auto& [s, x] : x_s) {
        if (0 + EPS < x and x < 1 - EPS) {
            LOG_F(INFO, "not integer %f", x);
            return false;
        }
    }
    return true;
}

void maximal_set(const Graph& g, node_set& s)
{
    vector<bool> visited(g.get_n(), false);
    for (const auto& v : s) {
        visited[v] = true;
        for_adj(g, v, n) {
            visited[n] = true;
        }
    }
    // while some one is not visited
    // get the one with the least degree
    // add to the set and mark him and its neighbors as visited
    while (find(visited.begin(), visited.end(), false) != visited.end()) {
        int min_degree = g.get_n();
        int min_v = -1;
        for (int v = 0; v < g.get_n(); v++) {
            if (visited[v]) {
                continue;
            }
            if (g.get_degree(v) < min_degree) {
                min_degree = g.get_degree(v);
                min_v = v;
            }
        }
        s.insert(min_v);
        visited[min_v] = true;
        for_adj(g, min_v, n) {
            visited[n] = true;
        }
    }
}

bool is_all_active(const Graph& g, const node_set& s)
{
    for (const auto& v : s) {
        if (!g.is_active(v)) {
            LOG_F(ERROR, "Node %d is not active but is in a set.", v);
            return false;
        }
    }
    return true;
}

bool check_indep_sets(const Graph& g, const vector<node_set>& indep_sets)
{
    for (const node_set& set : indep_sets) {
        if (not is_all_active(g, set)) {
            return false;
        }
        for (node const u : set) {
            for (node const v : set) {
                if (g.get_adjacency(u, v) != 0) {
                    LOG_F(ERROR, "%d %d are not independent", u, v);
                    return false;
                }
            }
        }
    }
    return true;
}
