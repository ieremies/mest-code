
#include <algorithm>
#include <fstream>
#include <iostream>

#include "../incl/utils.hpp"

#define MAX_GENERATED_SET 100

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
                  const vector<node_set>& indep_sets,
                  map<node_set, cost>& x_s,
                  const cost& sol)
{
    vector<node_set> sets_sol;
    for (node_set const& set : indep_sets) {
        if (x_s[set] >= 1 - EPS) {
            sets_sol.push_back(set);
        }
    }
    g.apply_changes_to_sol(sets_sol);
    string log = "SOL: %f = ";
    for (const auto& s : sets_sol) {
        log += to_string(s) + " ";
    }
    LOG_F(INFO, log.c_str(), sol);
}

bool integral(const map<node_set, cost>& x_s)
{
    for (const auto& [s, x] : x_s) {
        if (0 + EPS < x and x < 1 - EPS) {
            LOG_F(INFO, "Not integer %Lf", x);
            return false;
        }
    }
    return true;
}

void maximal_set(const Graph& orig, node_set& s)
{
    Graph g = Graph(orig);
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

bool check_indep_set(const Graph& g, const node_set& set)
{
    for (node const u : set) {
        for (node const v : set) {
            if (g.get_adjacency(u, v) != 0) {
                return false;
            }
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
        if (!check_indep_set(g, set)) {
            return false;
        }
    }
    return true;
}

void enrich(const Graph& g, vector<node_set>& indep_sets)
{
    LOG_SCOPE_FUNCTION(INFO);
    for (node_set set : indep_sets) {
        maximal_set(g, set);
    }
    // int i = 0;
    // for_nodes(g, u) {
    //     node_set s = {u};
    //     maximal_set(g, s);
    //     indep_sets.push_back(s);
    //     i++;
    //     if (i > MAX_GENERATED_SET) {
    //         break;
    //     }
    // }
}

Graph* read_dimacs_instance(const string& filename)
{
    ifstream infile(filename);
    string line;
    int n_vertices = 0;
    int m_edges = 0;

    do {
        getline(infile, line);
    } while (line[0] != 'p');

    // format line
    char _[10];
    sscanf(line.c_str(), "p %s %d %d", _, &n_vertices, &m_edges);
    auto* g = new Graph(n_vertices);

    // add edges to the graph
    int zero_indexed = 1;
    vector<pair<int, int>> edges;
    for (int i = 0; i < m_edges; i++) {
        int u = 0;
        int v;
        getline(infile, line);
        (void)sscanf(line.c_str(), "e %d %d", &u, &v);
        if (u == 0 or v == 0) {
            zero_indexed = 0;
        }
        edges.push_back({u, v});
    }

    // add edges to the graph
    for (const auto& [u, v] : edges) {
        g->add_edge(u - zero_indexed, v - zero_indexed);
    }

    LOG_F(INFO,
          "Read instance with %d vertexes and %lu edges",
          g->get_n(),
          g->get_m());
    return g;
}

void log_graph_stats(const Graph& g, const string& name)
{
    LOG_F(INFO,
          "%s: %d nodes, %lu edges, density %.2f, max degree: %d.",
          name.c_str(),
          g.get_active_n(),
          g.get_m(),
          (g.get_m() / (g.get_n() * (g.get_n() - 1) / 2.0)) * 100,
          g.get_degree(g.get_node_max_degree()));
}

bool check_connectivity(const Graph& g)
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

bool check_universal(const Graph& g)
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
