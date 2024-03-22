#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "branch_cut_price.hpp"
#include "graph.hpp"
#include "heuristic.hpp"
#include "utils.hpp"

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
    graph const g(argv[1]);

    color_sol const upper_bound = heuristic(g);
    LOG_F(INFO, "%s", upper_bound.str(g));
    // enrich(*g, indep_sets);
    // TODO quando a instância for densa, usar clique (mas isso é raro)
    // TODO usar independance number para achar um lb

    const color_sol res = branch_cut_price::solve(g, upper_bound);

    LOG_F(INFO, "Solved with: %f", res.cost);

    return 0;
}
