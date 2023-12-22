#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../incl/branch.hpp"
#include "../incl/graph.hpp"
#include "../incl/heuristic.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"

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

int main(int argc, char** argv)
{
    // Logging config
    loguru::g_preamble_date = false;
    loguru::g_preamble_thread = false;
    loguru::g_preamble_time = false;
    loguru::g_stderr_verbosity = 0;
    loguru::init(argc, argv);
    loguru::add_file(
        "log.log", loguru::FileMode::Truncate, loguru::Verbosity_INFO);

    // Read the instance and create the graph
    Graph* g = read_dimacs_instance(argv[1]);

    vector<node_set> indep_sets;
    cost upper_bound = heuristic(*g, indep_sets);
    cost lower_bound = 0;
    enrich(*g, indep_sets);

    cost sol = 0;
    map<node_set, double> x_s;  // x[s] = 1 if s is in the solution

    Branch tree;

    do {
        x_s.clear();
        sol = Solver::get_instance().solve(*g, indep_sets, x_s);
        LOG_F(INFO, "Solved with value %Lf", sol);

        if (lower_bound == 0) {
            lower_bound = sol;
            if (lower_bound + 1 > upper_bound) {
                break;
            }
        }

        if (integral(x_s) and sol + EPS < upper_bound) {
            upper_bound = sol;
            log_solution(*g, x_s, sol);
        } else {
            tree.branch(*g, indep_sets, x_s, sol);
        }

        indep_sets = tree.next(*g, upper_bound);

    } while (!indep_sets.empty());

    LOG_F(WARNING, "Upper bound: %Lf", upper_bound);

    delete g;

    return 0;
}
