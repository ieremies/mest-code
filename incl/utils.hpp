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

#include "../incl/graph.hpp"
#include "../lib/loguru.hpp"

#define EPS 1e-9

#define HANDLE_GRB_EXCEPTION(instruction) \
    try { \
        instruction; \
    } catch (GRBException & e) { \
        ABORT_F("GRB_exception = %d (%s)", \
                e.getErrorCode(), \
                e.getMessage().c_str()); \
    }

using namespace std;
using color = unsigned int;
using cost = long double;

string to_string(const node_set&);
void log_solution(const Graph& g,
                  const vector<node_set>& indep_sets,
                  map<node_set, cost>& x_s,
                  const cost& sol);
bool integral(const map<node_set, cost>&);
void maximal_set(const Graph&, node_set&);
bool check_indep_sets(const Graph&, const vector<node_set>&);
void enrich(const Graph& g, vector<node_set>& indep_sets);

inline string to_string(const mod_type& t)
{
    return t == mod_type::contract ? "contract" : "conflict";
}

#endif  // UTILS_H
