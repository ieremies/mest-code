#ifndef UTILS_H
#define UTILS_H

#include <map>
#include <set>
#include <string>

#define LOGURU_SCOPE_TIME_PRECISION 9
#define LOGURU_FILENAME_WIDTH 15
#ifdef NDEBUG
#    define LOGURU_VERBOSE loguru::Verbosity_WARNING
#else
#    define LOGURU_VERBOSE loguru::Verbosity_MAX
#endif

#include <loguru.hpp>

#include "../incl/graph.hpp"

#define EPS 1e-9
#define TIMELIMIT 3600

#ifdef NDEBUG
#    define SINGLE_THREAD true
#else
#    define SINGLE_THREAD false
#endif

using namespace std;
using node_set = set<Graph::node>;
using node = Graph::node;
using edge = Graph::edge;
using color = unsigned int;
using cost = long double;

string to_string(const node_set&);
void log_solution(const Graph& g,
                  const map<node_set, double>& x_s,
                  const double& sol);

inline auto to_string(const mod_type& t)
{
    return t == mod_type::contract ? "contract" : "conflict";
}

#endif  // UTILS_H
