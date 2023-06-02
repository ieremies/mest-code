#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../incl/branch.hpp"
#include "../incl/graph.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"

// Start counting time limit
const chrono::seconds time_limit(600);
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
    sscanf(line.c_str(), "p edge %d %d", &n_vertices, &m_edges);
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

/*
** Defines the initial independent sets of g and place them in
** vector<node_set> indep_sets.
*/
void initial_sets(Graph& g, vector<node_set>& indep_sets)
{
    // Trivial independent sets
    for (node n = 0; n < g.get_n(); ++n) {
        node_set s;
        s.insert(n);
        indep_sets.push_back(s);
    }
}

/*
** Defines the initial upper bound.
*/
int primal_heuristic(const Graph&)
{
    return 10000000;
}

bool integral(const map<node_set, double>& x_s)
{
    for (auto& [s, x] : x_s) {
        if (0 + EPS < x and x < 1 - EPS) {
            return false;
        }
        if (x > 0 + EPS) {
            LOG_F(INFO, "x[%s] = %f", to_string(s).c_str(), x);
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    // Logging config
    loguru::init(argc, argv);
    loguru::add_file(
        "log.log", loguru::FileMode::Truncate, loguru::Verbosity_MAX);

    // Read the instance and create the graph
    Graph* g = read_dimacs_instance(argv[1]);

    int upper_bound = primal_heuristic(*g);

    vector<node_set> indep_sets;
    initial_sets(*g, indep_sets);

    double sol = 0;  // place holder
    map<node_set, double> x_s;  // x[s] = 1 if s is in the solution

    // Create the branching tree with the initial node
    Branch tree;

    do {
        x_s.clear();
        // g->log();
        sol = Solver().solve(*g, indep_sets, x_s);
        LOG_F(INFO, "Solved with value %f", sol);

        if (integral(x_s) and sol < upper_bound) {
            LOG_F(WARNING, "Updating upper bound to %f", sol);
            // break;
            upper_bound = sol;
        } else {
            tree.branch(*g, indep_sets, x_s, sol);
        }

        indep_sets = tree.next(*g, upper_bound);

    } while (!indep_sets.empty() and !check_time());

    LOG_F(INFO, "Time out?: %d", check_time());
    LOG_F(INFO, "Upper bound: %d", upper_bound);
    delete g;

    return 0;
}
