#include <algorithm>
#include <iostream>
#include <string>

#include "utils.hpp"

bool color_sol::is_integral() const
{
    for (const auto& [s, x] : x_sets) {
        if (0 + EPS < x and x < 1 - EPS) {
            return false;
        }
    }
    return true;
}

string color_sol::to_string(const graph& g) const
{
    vector<node_set> sets_used;
    for (const auto& [s, x] : x_sets) {
        if (x > 0 + EPS) {
            sets_used.push_back(s);
        }
    }
    g.apply_changes_to_sol(sets_used);
    string res = "SOL: " + std::to_string(cost) + " = ";
    for (const auto& s : sets_used) {
        res += ::to_string(s) + " ";
    }
    return res;
}

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

bool check_indep_set(const graph& g, const node_set& s)
{
    for (const node& u : s) {
        if (not g.is_active(u)) {
            LOG_F(WARNING, "Node %d is not active.", u);
            return false;
        }
        for_adj(g, u, v) {
            if (s.count(v) > 0) {
                LOG_F(WARNING, "Nodes %d and %d are adjacent.", u, v);
                return false;
            }
        }
    }
    return true;
}

void maximal_set(const graph& orig, node_set& s)
{
    graph g = graph(orig);
    for (node const u : g.get_closed_neighborhood(s)) {
        g.deactivate(u);
    }

    while (not g.is_empty()) {
        node const n = g.get_node_min_degree();
        s.insert(n);
        for_adj(g, n, u) {
            g.deactivate(u);
        }
        g.deactivate(n);
    }
}

bool check_connectivity(const graph& g)
{
    if (not g.is_connected()) {
        LOG_F(WARNING, "Graph is not connected!.");
        return false;
    }
    if (not g.is_connected_complement()) {
        LOG_F(WARNING, "Graph complement is not connected!.");
        return false;
    }
    return true;
}

bool check_universal(const graph& g)
{
    bool res = false;
    for_nodes(g, u) {
        if (g.get_degree(u) == g.get_active_n() - 1) {
            LOG_F(WARNING, "Node %d is universal.", u);
            res = true;
        }
    }
    return res;
}
