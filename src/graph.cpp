#include <fstream>

#include "../incl/graph.hpp"

#include "../incl/utils.hpp"

// ==================== graph ====================
graph::graph(int nnodes)
    : n(nnodes)
    , m(0)
    , deg(n, 0)
    , adj(n, vector<node>(n, 0))
    , adj_bool(n, 0)
    , weights(n, 0.0)
{
    active.set();
}

graph::graph(const string& filename)
    : n(0)
    , m(0)
{
    active.set();

    ifstream infile(filename);
    string line;

    do {
        getline(infile, line);
    } while (line[0] != 'p');

    // format line
    char _[10];
    sscanf(line.c_str(), "p %s %hd %ld", _, &n, &m);

    adj = vector<vector<node>>(n, vector<node>(n, 0));
    deg = vector<node>(n, 0);
    adj_bool = vector<bitset<max_nodes>>(n, 0);
    weights = vector<double>(n, 0.0);

    // add edges to the graph
    int zero_indexed = 1;
    vector<pair<int, int>> edges;
    for (unsigned long i = 0; i < m; i++) {
        int u = 0;
        int v = 0;
        getline(infile, line);
        (void)sscanf(line.c_str(), "e %d %d", &u, &v);
        if (u == 0 or v == 0) {
            zero_indexed = 0;
        }
        edges.emplace_back(u, v);
    }

    // add edges to the graph
    for (const auto& [u, v] : edges) {
        add_edge(u - zero_indexed, v - zero_indexed);
    }

    LOG_F(INFO, "Graph created with %d nodes and %lu edges.", n, m);
}

// copy constructor
graph::graph(const graph& g) = default;

graph::node graph::get_n() const
{
    return n;
}

graph::node graph::get_active_n() const
{
    node count = 0;
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            count++;
        }
    }
    return count;
}

unsigned long int graph::get_m() const
{
    // FIXME I can save this info and not recompute every time.
    unsigned long int count = 0;
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            for (node j = 0; j < n; j++) {
                if (active[j]) {
                    count += 1;
                }
            }
        }
    }
    return count / 2;
}

bool graph::is_empty() const
{
    for (node i = 0; i < n; i++) {
        if (active[i]) {
            return false;
        }
    }
    return true;
}

graph::node graph::check_deg(node u) const
{
    node count = 0;
    for (node v = 0; v < n; v++) {
        if (get_adjacency(u, v) > 0) {
            count++;
        }
    }
    return count;
}

graph::node graph::get_degree(node u) const
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

bool graph::check_all_deg() const
{
    for (node u = 0; u < n; u++) {
        (void)get_degree(u);
    }
    return true;
}

graph::node graph::get_node_max_degree() const
{
    node max = 0;
    node max_deg = 0;
    for (node u = 0; u < n; u++) {
        if (not is_active(u)) {
            continue;
        }
        node const deg_u = get_degree(u);
        if (deg_u >= max_deg) {
            max = u;
            max_deg = deg_u;
        }
    }
    return max;
}

graph::node graph::get_node_min_degree() const
{
    node min = 0;
    node min_deg = n;
    for (node u = 0; u < n; u++) {
        if (not is_active(u)) {
            continue;
        }
        node const deg_u = get_degree(u);
        if (deg_u <= min_deg) {
            min = u;
            min_deg = deg_u;
        }
    }
    return min;
}

graph::node graph::get_adjacency(node u, node v) const
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
    DCHECK_F(adj_bool[u][v] == adj_bool[v][u],
             "Adjacency of %d %d (active) nodes are not correct in bool.",
             u,
             v);
    return adj[u][v];
}

void graph::do_conflict(node u, node v)
{
    LOG_F(INFO, "Doing conflict %d %d.", u, v);
    add_edge(u, v);
}

void graph::undo_conflict(node u, node v)
{
    LOG_F(INFO, "Undoing conflict %d %d.", u, v);
    remove_edge(u, v);
}

void graph::deactivate(node u)
{
    if (not is_active(u)) {
        return;  // nothing to do
    }
    // LOG_F(INFO, "Deactivating node %d.", u);
    // adj[u] should not change
    for (node v = 0; v < n; v++) {
        // if v is inactive, adj[u][v] is 0;
        // therefore this will not act in a inative node.
        if (adj_bool[u][v]) {
            DCHECK_F(is_active(v), "Changing adjacency of inactive node.");
            deg[v]--;
            m--;
            adj[v][u] = 0;
            adj_bool[v][u] = false;
        }
    }

    active[u] = false;
    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void graph::do_contract(node u, node v)
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
            adj_bool[u][w] = true;
            adj_bool[w][u] = true;
        }
        adj[u][w] += adj[v][w];
        adj[w][u] += adj[v][w];

        adj[w][v] = 0;
        adj_bool[w][v] = false;
        deg[w]--;
    }

    if (adj[u][v] > 0) {
        deg[u]--;
        adj[u][v] = 0;
        adj_bool[u][v] = false;
    }

    active[v] = false;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void graph::undo_contract(node u, node v)
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
            adj_bool[u][w] = false;
            adj_bool[w][u] = false;
        }

        adj[w][v] = adj[v][w];
        deg[w]++;
        adj_bool[w][v] = true;
    }

    if (adj[u][v] > 0) {
        deg[u]++;
        adj[u][v] = adj[v][u];
        adj_bool[u][v] = true;
    }

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
}

void graph::change(mod_type t, node u, node v)
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

void graph::undo(mod_type tc, node uc, node vc)
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

unsigned long int graph::add_edge(node u, node v)
{
    DCHECK_F(
        active[u] && active[v], "Interacting with inactive nodes %d %d", u, v);
    DCHECK_F(u != v, "Cannot act with equal nodes.");

    if (adj[u][v]++ == 0) {
        deg[u]++;
        adj_bool[u][v] = true;
    }
    if (adj[v][u]++ == 0) {
        deg[v]++;
        adj_bool[v][u] = true;
    }
    m++;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
    return m;
}

unsigned long int graph::remove_edge(node u, node v)
{
    DCHECK_F(active[u] && active[v], "Interacting with inactive nodes");
    DCHECK_F(u != v, "Cannoct act with equal nodes.");
    DCHECK_F(adj[u][v] > 0, "Edge does not exists.");

    if (--adj[u][v] == 0) {
        deg[u]--;
        adj_bool[u][v] = false;
    }
    if (--adj[v][u] == 0) {
        deg[v]--;
        adj_bool[v][u] = false;
    }
    m--;

    DCHECK_F(check_all_deg(), "Degree is not consistent.");
    return m;
}

graph::node graph::first_act_node() const
{
    return next_act_node(-1);
}

graph::node graph::next_act_node(node u) const
{
    for (node i = u + 1; i < n; i++) {
        if (is_active(i)) {
            return i;
        }
    }
    return n;
}

void graph::log() const
{
    log_stats("Graph");
    for (node u = 0; u < n; u++) {
        if (!is_active(u)) {
            continue;
        }
        // Node %d : [deg: %d | adj: %s]
        string s = "Node " + to_string(u)
            + " : [deg: " + to_string(get_degree(u)) + " | adj: ";
        for (node v = 0; v < n; v++) {
            if (get_adjacency(u, v) > 0) {
                s += to_string(v) + " ";
            }
        }
        s += "]";
        LOG_F(INFO, "%s", s.c_str());
    }
}

void graph::log_stats(const string& name) const
{
    LOG_F(INFO,
          "%s: %d nodes, %lu edges, density %.2f, max degree: %d.",
          name.c_str(),
          get_active_n(),
          get_m(),
          (get_m() / (get_n() * (get_n() - 1) / 2.0)) * 100,
          get_degree(get_node_max_degree()));
}

void graph::apply_changes_to_sol(vector<node_set>& indep_sets) const
{
    // iterate over delta in reverse order
    for (auto it = delta.rbegin(); it != delta.rend(); ++it) {
        auto [t, u, v] = *it;
        if (t == mod_type::conflict) {
            continue;
        }
        for (auto set : indep_sets) {
            if (set.find(u) != set.end()) {
                set.insert(v);
                break;
            }
        }
    }
}

graph::node graph::first_adj_node(node u) const
{
    return next_adj_node(u, -1);
}

graph::node graph::next_adj_node(node u, node v) const
{
    for (node i = v + 1; i < n; i++) {
        if (get_adjacency(u, i) > 0) {
            return i;
        }
    }
    return n;
}

graph::edge graph::first_edge() const
{
    return next_edge({-1, -1});
}

graph::edge graph::next_edge(edge e) const
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
// ==================== Neighborhood function ====================
node_set graph::get_closed_neighborhood(const node_set& s) const
{
    bitset<max_nodes> ret(0);
    // do the OR of adj_bool[u] for all u in s
    for (const auto& u : s) {
        ret |= adj_bool[u];
    }
    for (const auto& u : s) {
        ret[u] = true;
    }
    node_set ret_set;
    for (node i = 0; i < n; i++) {
        if (ret[i]) {
            ret_set.insert(i);
        }
    }
    return ret_set;
}

node_set graph::get_closed_neighborhood(const node& u) const
{
    node_set const ret = {u};
    return get_closed_neighborhood(ret);
}

node_set graph::get_open_neighborhood(const node_set& s) const
{
    bitset<max_nodes> ret(0);
    // do the OR of adj_bool[u] for all u in s
    for (const auto& u : s) {
        ret |= adj_bool[u];
    }
    for (const auto& u : s) {
        ret[u] = false;
    }
    node_set ret_set;
    for (node i = 0; i < n; i++) {
        if (ret[i]) {
            ret_set.insert(i);
        }
    }
    return ret_set;
}

node_set graph::get_open_neighborhood(const node& u) const
{
    node_set const ret = {u};
    return get_open_neighborhood(ret);
}

void graph::k_core(int k)
{
    bool changed = true;
    while (changed) {
        changed = false;
        for (node u = 0; u < n; u++) {
            if (not is_active(u)) {
                continue;
            }
            if (get_degree(u) < k - 1) {
                LOG_F(INFO,
                      "k-core reduction %d (deg: %d | lb: %d).",
                      u,
                      get_degree(u),
                      k);
                deactivate(u);
                changed = true;
            }
        }
    }
}

// Transform the graph into its complement
void graph::complement()
{
    for (node u = 0; u < n; u++) {
        if (not is_active(u)) {
            continue;
        }
        adj_bool[u].flip();
    }
}

bool graph::is_connected() const
{
    bitset<max_nodes> visited(0);
    node_set to_visit;
    for (node u = 0; u < n; u++) {
        if (is_active(u)) {
            to_visit.insert(u);
            break;
        }
    }
    while (not to_visit.empty()) {
        node const u = *to_visit.begin();
        to_visit.erase(u);
        visited[u] = true;
        for (node v = 0; v < n; v++) {
            if (get_adjacency(u, v) > 0 and not visited[v]) {
                to_visit.insert(v);
            }
        }
    }
    return visited.count() == get_active_n();
}

bool graph::is_connected_complement() const
{
    graph g(*this);
    g.complement();
    return g.is_connected();
}

void graph::set_weight(const node& u, const double& w)
{
    weights[u] = w;
}

double graph::get_weight(const node& u) const
{
    return weights[u];
}

double graph::get_weight(const node_set& s) const
{
    double w = 0;
    for (const auto& u : s) {
        w += get_weight(u);
    }
    return w;
}
