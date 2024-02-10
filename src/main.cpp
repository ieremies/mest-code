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

void config_logging(int argc, char** argv)
{
    loguru::g_preamble_date = false;
    loguru::g_preamble_thread = false;
    loguru::g_preamble_time = false;
    loguru::g_stderr_verbosity = 0;
    loguru::init(argc, argv);
    loguru::add_file(
        "log.log", loguru::FileMode::Truncate, loguru::Verbosity_INFO);
}

int main(int argc, char** argv)
{
    // Logging config
    config_logging(argc, argv);

    // Read the instance and create the graph
    Graph* g = read_dimacs_instance(argv[1]);

    vector<node_set> indep_sets;
    cost upper_bound = heuristic(*g, indep_sets);
    enrich(*g, indep_sets);

    Solver solver = Solver();
    Branch tree;

    while (!indep_sets.empty()) {
        map<node_set, cost> x_s;

        cost const sol = solver.solve(*g, indep_sets, x_s);
        LOG_F(INFO, "Solved with value %Lf", sol);

        if (integral(x_s) and sol + EPS < upper_bound) {
            upper_bound = sol;
            log_solution(*g, indep_sets, x_s, sol);
        }

        if (ceil(sol) < upper_bound) {
            tree.branch(*g, indep_sets, x_s, sol);
        }

        indep_sets = tree.next(*g, upper_bound);
    }

    LOG_F(WARNING, "Solved with: %Lf", upper_bound);

    delete g;

    return 0;
}
