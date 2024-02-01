#include <fstream>

#include "../incl/graph.hpp"

#include "../incl/utils.hpp"

// ==================== Graph ====================
Graph::Graph(int nnodes)
    : n(nnodes)
    , m(0)
    , delta()
    , deg(n, 0)
    , active(n, true)
    , adj(n, vector<node>(n, 0))
{
}

Graph::node Graph::get_n() const
{
    // return n;
    int count = 0;
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            count++;
        }
    }
    return count;
}

unsigned long int Graph::get_m() const
{
    // return m;
    unsigned long int count = 0;
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            for (node j = 0; j < n; j++) {
                if (active[j]) {
                    count += adj[i][j];
                }
            }
        }
    }
    return count / 2;
}

// copy constructor
Graph::Graph(const Graph& g)
    : n(g.n)
    , m(g.m)
    , delta(g.delta)
    , deg(g.deg)
    , active(g.active)
    , adj(g.adj)
{
}

Graph::node Graph::check_deg(node u) const
{
    node count = 0;
    for (node v = 0; v < get_n(); v++) {
        if (get_adjacency(u, v) > 0) {
            count++;
        }
    }
    return count;
}

bool Graph::is_empty() const
{
    for (node u = 0; u < n; u++) {
        if (is_active(u)) {
            return false;
        }
    }
    return true;
}

Graph::node Graph::get_degree(node u) const
{
    if (not is_active(u)) {
        return 0;
    }
    DCHECK_F(deg[u] == check_deg(u),
             "Degree is not consistent (%d)\n%s.",
             u,
             loguru::stacktrace().c_str());
    return deg[u];
}

Graph::node Graph::max_degree() const
{
    node max = 0;
    for (node u = 0; u < n; u++) {
        if (get_degree(u) > get_degree(max)) {
            max = u;
        }
    }
    return max;
}

bool Graph::is_active(node u) const
{
    return active[u];
}

Graph::node Graph::get_adjacency(node u, node v) const
{
    if (not is_active(u) or not is_active(v) or u == v) {
        return 0;
    }
    DCHECK_F(
        adj[u][v] == adj[v][u],
        "Adjacency of %d %d (active) nodes are not mirrowed (%d | %d).\n%s",
        u,
        v,
        adj[u][v],
        adj[v][u],
        loguru::stacktrace().c_str());
    return adj[u][v];
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

void Graph::deactivate(node v)
{
    if (not is_active(v)) {
        return;
    }
    LOG_F(INFO, "Deactivating node %d.", v);
    // adj[v] should not change
    for (node u = 0; u < n; u++) {
        if (is_active(u) and get_adjacency(u, v) > 0) {
            adj[u][v] = 0;
            deg[u]--;
        }
    }

    active[v] = false;
}

void Graph::activate(node v)
{
    if (is_active(v)) {
        return;
    }
    LOG_F(INFO, "Activating node %d", v);
    // adj[v] should have been kept untouched.
    for (node u = 0; u < n; u++) {
        if (is_active(u) and adj[v][u] > 0) {
            adj[u][v] = adj[v][u];
            deg[u]++;
        }
    }
    active[v] = true;
}

void Graph::do_contract(node u, node v)
{
    LOG_F(INFO, "Doing contract %d %d.", u, v);
    for (node n1 = 0; n1 < n; n1++) {
        if (get_adjacency(v, n1) == 0) {
            continue;
        }

        if (adj[u][n1] == 0) {
            deg[u]++;
        }
        if (adj[n1][v] > 0) {
            deg[u]--;
        }
        adj[u][n1] += adj[n1][v];
        adj[n1][u] += adj[n1][v];
        adj[n1][v] = 0;
        adj[u][v] = 0;
    }
    active[v] = false;
}

void Graph::undo_contract(node u, node v)
{
    LOG_F(INFO, "Undoing contract %d %d.", u, v);
    active[v] = true;
    for (node n1 = 0; n1 < n; n1++) {
        if (not is_active(n1) or adj[v][n1] == 0) {
            continue;
        }
        if (adj[u][n1] == adj[v][n1]) {
            deg[u]--;
        }
        if (adj[n1][v] == 0) {
            deg[u]++;
        }
        adj[u][n1] -= adj[v][n1];
        adj[n1][u] -= adj[v][n1];
        adj[n1][v] = adj[v][n1];
        adj[u][v] = adj[v][u];
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
    CHECK_F(
        active[u] && active[v], "Interacting with inactive nodes %d %d", u, v);
    CHECK_F(u != v, "Cannoct act with equal nodes.");

    if (adj[u][v]++ == 0) {
        deg[u]++;
    }
    if (adj[v][u]++ == 0) {
        deg[v]++;
    }
    m++;

    return m;
}

unsigned long int Graph::remove_edge(node u, node v)
{
    CHECK_F(active[u] && active[v], "Interacting with inactive nodes");
    CHECK_F(u != v, "Cannoct act with equal nodes.");
    CHECK_F(adj[u][v] > 0, "Edge does not exists.");

    if (--adj[u][v] == 0) {
        deg[u]--;
    }
    if (--adj[v][u] == 0) {
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
            if (get_adjacency(u, v) > 0) {
                LOG_F(INFO, "     |-- %d: %d", v, adj[u][v]);
            }
        }
    }
}

void Graph::apply_changes_to_sol(vector<node_set>& indep_sets) const
{
    // iterate over delta in reverse order
    for (auto it = delta.rbegin(); it != delta.rend(); ++it) {
        auto [t, u, v] = *it;
        if (t != mod_type::contract) {
            continue;
        }
        for (auto set : indep_sets) {
            if (set.find(v) != set.end()) {
                set.erase(v);
                break;
            }
        }
    }
}

Graph::node Graph::first_adj_node(node u) const
{
    return next_adj_node(u, -1);
}

Graph::node Graph::next_adj_node(node u, node v) const
{
    for (node i = v + 1; i < n; i++) {
        if (get_adjacency(u, i) > 0) {
            return i;
        }
    }
    return n;
}

Graph::edge Graph::first_edge() const
{
    return next_edge({-1, -1});
}

Graph::edge Graph::next_edge(edge e) const
{
    for (node u = e.first; u < n; u++) {
        for (node v = e.second + 1; v < n; v++) {
            if (get_adjacency(u, v) > 0) {
                return {u, v};
            }
        }
    }
    return {n, n};
}
