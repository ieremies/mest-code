
#include <algorithm>
#include <fstream>
#include <iostream>

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
    LOG_F(WARNING, log.c_str(), sol);
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

void maximal_set(const Graph& g, node_set& s)
{
    // This function can extend to be a maximal, but it needs to take
    // care into adding vertexis that may incur in a cut, subtracting
    // the cuts dual variable value and making it not vialoted (>1).
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

/*
** The idea is to create some usefull independent sets.
** - maximal independent set starting with each vertex
** - maximal independent set starting with a pair of non-adj vertexes.
** - remove one element from a independnet set
**    not sure what
*/
void enrich(const Graph& g, vector<node_set>& indep_sets)
{
    for (node_set set : indep_sets) {
        maximal_set(g, set);
    }
    for_nodes(g, u) {
        node_set s = {u};
        maximal_set(g, s);
        indep_sets.push_back(s);
    }
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
    for (int i = 0; i < m_edges; i++) {
        int u = 0;
        int v;
        getline(infile, line);
        (void)sscanf(line.c_str(), "e %d %d", &u, &v);
        // if filename ends with .col, then the vertices are numbered from 1
        if (filename.substr(filename.size() - 4) == ".col") {
            g->add_edge(u - 1, v - 1);
        } else {
            g->add_edge(u, v);
        }
    }
    LOG_F(INFO,
          "Read instance with %d vertexes and %lu edges",
          g->get_n(),
          g->get_m());
    return g;
}
