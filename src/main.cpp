#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define LOGURU_SCOPE_TIME_PRECISION 9

#include "../incl/branch.hpp"
#include "../incl/graph.hpp"
#include "../incl/heuristic.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"

// Start counting time limit
const chrono::seconds time_limit(TIMELIMIT);
const auto start_time = chrono::steady_clock::now();

bool check_time()
{
    // check time limit
    const auto current_time = chrono::steady_clock::now();
    const auto elapsed_time =
        chrono::duration_cast<chrono::seconds>(current_time - start_time);
    return elapsed_time > time_limit;
}

/*
** Function to read a DIMACS instanc from file (filename) and
** create the Graph g.
*/
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
        g->add_edge(u - 1, v - 1);
    }
    LOG_F(INFO,
          "Read instance with %lu vertexes and %d edges",
          g->get_m(),
          g->get_n());
    return g;
}

bool integral(const map<node_set, double>& x_s)
{
    for (const auto& [s, x] : x_s) {
        if (0 + EPS < x and x < 1 - EPS) {
            LOG_F(INFO, "not integer %f", x);
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    // Logging config
    // loguru::g_preamble_time = false;
    loguru::g_preamble_date = false;
    loguru::g_preamble_thread = false;
    loguru::init(argc, argv);
    loguru::add_file("log.log", loguru::FileMode::Truncate, 0);

    // Read the instance and create the graph
    Graph* g = read_dimacs_instance(argv[1]);

    vector<node_set> indep_sets;
    cost upper_bound = dsatur(*g, indep_sets);
    cost lower_bound = 0;

    cost sol = 0;
    map<node_set, double> x_s;  // x[s] = 1 if s is in the solution

    Branch tree;

    do {
        x_s.clear();
        // g->log();
        sol = Solver().solve(*g, indep_sets, x_s);
        LOG_F(INFO, "Solved with value %Lf", sol);

        if (integral(x_s)) {
            if (sol < upper_bound + EPS) {
                upper_bound = sol;
                log_solution(*g, x_s, sol);
            }
            if (sol - 1 < lower_bound + EPS) {
                LOG_F(INFO, "Gap is closed! %Lf %Lf", lower_bound, upper_bound);
                break;
            }
        } else {
            tree.branch(*g, indep_sets, x_s, sol);
        }

        indep_sets = tree.next(*g, upper_bound, lower_bound);

    } while (!indep_sets.empty() and !check_time());

    LOG_F(INFO, "Timed out?: %d", check_time());
    LOG_F(INFO, "Upper bound: %Lf", upper_bound);

    delete g;

    return 0;
}
