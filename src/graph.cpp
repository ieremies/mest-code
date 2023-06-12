#include <fstream>

#include "../incl/graph.hpp"

#include "../lib/loguru.hpp"

// ==================== Graph ====================
Graph::Graph(int nnodes)
    : n(nnodes)
    , m(0)
    , delta()
    , deg(n, 0)
    , active(n, true)
    , inc(n, vector<node>(n, 0))
{
}

Graph::node Graph::get_n() const
{
    return n;
}

unsigned long int Graph::get_m() const
{
    return m;
}

Graph::node Graph::check_deg(node u) const
{
    node count = 0;
    for (node v = 0; v < get_n(); v++) {
        if (get_incidency(u, v) > 0) {
            count++;
        }
    }
    return count;
}

Graph::node Graph::get_degree(node u) const
{
    if (not is_active(u)) {
        return 0;
    }
    CHECK_F(deg[u] == check_deg(u), "Degree is not consistent.");
    return deg[u];
}

bool Graph::is_active(node u) const
{
    return active[u];
}

Graph::node Graph::get_incidency(node u, node v) const
{
    if (not is_active(u) or not is_active(v)) {
        return 0;
    }
    CHECK_F(inc[u][v] == inc[v][u],
            "Incidency of %d %d (active) nodes are not mirrowed.",
            u,
            v);
    return inc[u][v];
}

void Graph::do_conflict(node u, node v)
{
    LOG_F(INFO, "Doing conflict %d %d.", u, v);
    add_edge(u, v);
}

void Graph::undo_conflict(node u, node v)
{
    LOG_F(INFO, "Undoing conflict %d %d.", u, v);
    remove_edge(u, v);
}

void Graph::do_contract(node u, node v)
{
    LOG_F(INFO, "Doing contract %d %d.", u, v);
    for (node n1 = 0; n1 < n; n1++) {
        if (get_incidency(v, n1) == 0) {
            continue;
        }

        if (inc[u][n1] == 0) {
            deg[u]++;
        }
        if (inc[n1][v] > 0) {
            deg[u]--;
        }
        inc[u][n1] += inc[n1][v];
        inc[n1][u] += inc[n1][v];
        inc[n1][v] = 0;
        inc[u][v] = 0;
    }
    active[v] = false;
}

void Graph::undo_contract(node u, node v)
{
    LOG_F(INFO, "Undoing contract %d %d.", u, v);
    active[v] = true;
    for (node n1 = 0; n1 < n; n1++) {
        if (not is_active(n1) or inc[v][n1] == 0) {
            continue;
        }
        if (inc[u][n1] == inc[v][n1]) {
            deg[u]--;
        }
        if (inc[n1][v] == 0) {
            deg[u]++;
        }
        inc[u][n1] -= inc[v][n1];
        inc[n1][u] -= inc[v][n1];
        inc[n1][v] = inc[v][n1];
        inc[u][v] = inc[v][u];
    }
}

void Graph::change(mod_type t, node u, node v)
{
    CHECK_F(active[u] && active[v], "Interacting with inactive nodes.");
    CHECK_F(u != v, "Cannoct act with equal nodes.");

    if (t == mod_type::conflict) {
        do_conflict(u, v);
    }
    if (t == mod_type::contract) {
        do_contract(u, v);
    }
    delta.push_back(mod {t, u, v});
}

void Graph::undo(mod_type tc, node uc, node vc)
{
    CHECK_F(uc != vc, "Cannot act with equal nodes.");
    CHECK_F(active[uc], "Interacting with inactive node.");

    const auto [t, u, v] = delta.back();
    CHECK_F(tc == t and uc == u and vc == v, "Undoing in the wrong order.");

    if (t == mod_type::conflict) {
        undo_conflict(u, v);
    }
    if (t == mod_type::contract) {
        undo_contract(u, v);
    }
    delta.pop_back();
}

unsigned long int Graph::add_edge(node u, node v)
{
    CHECK_F(active[u] && active[v], "Interacting with inactive nodes");
    CHECK_F(u != v, "Cannoct act with equal nodes.");

    if (inc[u][v]++ == 0) {
        deg[u]++;
    }
    if (inc[v][u]++ == 0) {
        deg[v]++;
    }
    m++;

    return m;
}

unsigned long int Graph::remove_edge(node u, node v)
{
    CHECK_F(active[u] && active[v], "Interacting with inactive nodes");
    CHECK_F(u != v, "Cannoct act with equal nodes.");
    CHECK_F(inc[u][v] > 0, "Edge does not exists.");

    if (--inc[u][v] == 0) {
        deg[u]--;
    }
    if (--inc[v][u] == 0) {
        deg[v]--;
    }
    m--;

    return m;
}

Graph::node Graph::first_act_node() const
{
    return next_act_node(-1);
}

Graph::node Graph::next_act_node(node u) const
{
    for (node i = u + 1; i < n; i++) {
        if (is_active(i)) {
            return i;
        }
    }
    return n;
}

void Graph::log() const
{
    LOG_F(INFO, "Graph: %d nodes, %lu edges", n, m);
    for (node u = 0; u < n; u++) {
        if (!is_active(u)) {
            continue;
        }
        LOG_F(INFO, "Node %d: %d edges", u, deg[u]);
        for (node v = 0; v < n; v++) {
            if (!is_active(v)) {
                continue;
            }
            if (get_incidency(u, v) > 0) {
                LOG_F(INFO, "     |-- %d: %d", v, inc[u][v]);
            }
        }
    }
}

void Graph::log_changes() const
{
    string log = "Contracts: ";
    for (auto [t, u, v] : delta) {
        if (t == mod_type::contract) {
            log += "(" + to_string(u) + "," + to_string(v) + ") ";
        }
    }
    LOG_F(WARNING, "%s", log.c_str());
}

Graph::node Graph::first_adj_node(node u) const
{
    return next_adj_node(u, -1);
}

Graph::node Graph::next_adj_node(node u, node v) const
{
    for (node i = v + 1; i < n; i++) {
        if (get_incidency(u, i) > 0) {
            return i;
        }
    }
    return n;
}
