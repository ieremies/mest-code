#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "../incl/branch.hpp"
#include "../incl/graph.hpp"
#include "../incl/heuristic.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"

void config_logging(int argc, char** argv)
{
    loguru::g_preamble_date = false;
    loguru::g_preamble_thread = false;
    loguru::g_preamble_time = false;
    loguru::g_stderr_verbosity = 0;
    loguru::g_flush_interval_ms = 100;
    loguru::init(argc, argv);
    loguru::add_file(
        "log.log", loguru::FileMode::Truncate, loguru::Verbosity_MAX);
}

int main(int argc, char** argv)
{
    // Logging config
    config_logging(argc, argv);

    // Read the instance and create the graph
    Graph* g = read_dimacs_instance(argv[1]);

    vector<node_set> indep_sets;
    cost upper_bound = heuristic(*g, indep_sets);
    // enrich(*g, indep_sets);

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
