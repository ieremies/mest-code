#ifndef UTILS_H

#define UTILS_H

#include <map>
#include <set>
#include <string>

#define LOGURU_SCOPE_TIME_PRECISION 9
#define LOGURU_FILENAME_WIDTH 15
#ifdef NDEBUG
#    define TIMELIMIT 3600
#    define SINGLE_THREAD true
#    define LOGURU_VERBOSE loguru::Verbosity_WARNING
#else
#    define TIMELIMIT 120
#    define SINGLE_THREAD false
#    define LOGURU_VERBOSE loguru::Verbosity_MAX
#endif

#include <loguru.hpp>

#include "../incl/graph.hpp"

#define EPS 1e-9

using namespace std;
using color = unsigned int;
using cost = long double;

string to_string(const node_set&);
void log_solution(const Graph& g,
                  const map<node_set, double>& x_s,
                  const double& sol);
bool integral(const map<node_set, double>&);

inline auto to_string(const mod_type& t)
{
    return t == mod_type::contract ? "contract" : "conflict";
}

#endif  // UTILS_H
