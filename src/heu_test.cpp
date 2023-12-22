#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../incl/graph.hpp"
#include "../incl/heuristic.hpp"
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
    (void)heuristic(*g, indep_sets);

    delete g;

    return 0;
}
