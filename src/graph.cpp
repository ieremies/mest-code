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

Graph::node Graph::get_n() const
{
    return n;
}

unsigned long int Graph::get_m() const
{
    // FIXME I can save this info and not recompute every time.
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

bool Graph::is_empty() const
{
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            return false;
        }
    }
    return true;
}

Graph::node Graph::check_deg(node u) const
{
    node count = 0;
    for (node v = 0; v < n; v++) {
        if (get_adjacency(u, v) > 0) {
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
    DCHECK_F(deg[u] == check_deg(u),
             "Degree is not consistent (%d: %d != %d).",
             u,
             deg[u],
             check_deg(u));
    return deg[u];
}

Graph::node Graph::get_node_max_degree() const
{
    node max = 0;
    node max_deg = 0;
    for (node u = 0; u < n; u++) {
        node const deg_u = get_degree(u);
        if (deg_u > max_deg) {
            max = u;
            max_deg = deg_u;
        }
    }
    return max;
}

bool Graph::check_all_deg() const
{
    for (node u = 0; u < n; u++) {
        (void)get_degree(u);
    }
    return true;
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
    DCHECK_F(adj[u][v] == adj[v][u],
             "Adjacency of %d %d (active) nodes are not mirrowed (%d | %d).",
             u,
             v,
             adj[u][v],
             adj[v][u]);
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

void Graph::deactivate(node u)
{
    if (not is_active(u)) {
        return;  // nothing to do
    }
    LOG_F(INFO, "Deactivating node %d.", u);
    // adj[u] should not change
    for (node v = 0; v < n; v++) {
        // if v is inactive, adj[u][v] is 0;
        // therefore this will not act in a inative node.
        if (adj[u][v] > 0) {
            DCHECK_F(is_active(v), "Changing adjacency of inactive node.");
            deg[v]--;
            adj[v][u] = 0;
        }
    }

    active[u] = false;
    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void Graph::do_contract(node u, node v)
{
    // v will be deactivated and adj[v] should be kept unchanged.
    // u will be added all v's edges
    DCHECK_F(is_active(u) && is_active(v), "Interacting with inactive nodes.");
    LOG_F(INFO, "Doing contract %d %d.", u, v);
    for (node w = 0; w < n; w++) {
        if (get_adjacency(v, w) == 0 or w == u) {
            continue;
        }
        if (adj[u][w] == 0) {
            deg[u]++;
            deg[w]++;
        }
        adj[u][w] += adj[v][w];
        adj[w][u] += adj[v][w];

        adj[w][v] = 0;
        deg[w]--;
    }

    if (adj[u][v] > 0) {
        deg[u]--;
        adj[u][v] = 0;
    }

    active[v] = false;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void Graph::undo_contract(node u, node v)
{
    DCHECK_F(is_active(u), "Interacting with inactive node.");
    LOG_F(INFO, "Undoing contract %d %d.", u, v);
    active[v] = true;
    for (node w = 0; w < n; w++) {
        if (adj[v][w] == 0 or not is_active(w) or w == u) {
            continue;
        }
        adj[u][w] -= adj[v][w];
        adj[w][u] -= adj[v][w];

        if (adj[u][w] == 0) {
            deg[u]--;
            deg[w]--;
        }

        adj[w][v] = adj[v][w];
        deg[w]++;
    }

    if (adj[u][v] > 0) {
        deg[u]++;
        adj[u][v] = adj[v][u];
    }

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void Graph::change(mod_type t, node u, node v)
{
    DCHECK_F(active[u] && active[v], "Interacting with inactive nodes.");
    DCHECK_F(u != v, "Cannoct act with equal nodes.");

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
    DCHECK_F(uc != vc, "Cannot act with equal nodes.");
    DCHECK_F(active[uc], "Interacting with inactive node.");

    const auto [t, u, v] = delta.back();
    DCHECK_F(tc == t and uc == u and vc == v, "Undoing in the wrong order.");

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
    DCHECK_F(
        active[u] && active[v], "Interacting with inactive nodes %d %d", u, v);
    DCHECK_F(u != v, "Cannoct act with equal nodes.");

    if (adj[u][v]++ == 0) {
        deg[u]++;
    }
    if (adj[v][u]++ == 0) {
        deg[v]++;
    }
    m++;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
    return m;
}

unsigned long int Graph::remove_edge(node u, node v)
{
    DCHECK_F(active[u] && active[v], "Interacting with inactive nodes");
    DCHECK_F(u != v, "Cannoct act with equal nodes.");
    DCHECK_F(adj[u][v] > 0, "Edge does not exists.");

    if (--adj[u][v] == 0) {
        deg[u]--;
    }
    if (--adj[v][u] == 0) {
        deg[v]--;
    }
    m--;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
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

node_set Graph::get_closed_neighborhood(node_set& s) const
{
    node_set ret = s;
    for (const auto& u : s) {
        for (node v = 0; v < n; v++) {
            if (get_adjacency(u, v) > 0) {
                ret.insert(v);
            }
        }
    }
    return ret;
}

node_set Graph::get_open_neighborhood(node_set& s) const
{
    node_set ret = get_closed_neighborhood(s);
    for (const auto& u : s) {
        ret.erase(u);
    }
    return ret;
}
