#ifndef UTILS_H

#define UTILS_H

#include <algorithm>
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
using cost = long double;  // might not be necessary

// === DIMACS functions =======================================================
Graph* read_dimacs_instance(const string& filename);

// === Log functions ==========================================================
void log_solution(const Graph& g,
                  const vector<node_set>& indep_sets,
                  map<node_set, cost>& x_s,
                  const cost& sol);
void log_graph_stats(const Graph& g, const string& name);

// === Check functions ========================================================
bool integral(const map<node_set, cost>&);
bool check_indep_set(const Graph&, const node_set&);
bool check_indep_sets(const Graph&, const vector<node_set>&);
bool check_connectivity(const Graph&);
bool check_universal(const Graph&);

// === Graph functions ========================================================
void maximal_set(const Graph&, node_set&);
void enrich(const Graph& g, vector<node_set>& indep_sets);

// === String functions =======================================================
string to_string(const node_set&);
inline string to_string(const mod_type& t)
{
    return t == mod_type::contract ? "contract" : "conflict";
}

// === Template functions =====================================================
template<typename T>
inline constexpr std::set<T> set_intersection(const std::set<T>& a,
                                              const std::set<T>& b)
{
    std::set<T> inter;
    std::set_intersection(a.begin(),
                          a.end(),
                          b.begin(),
                          b.end(),
                          std::inserter(inter, inter.begin()));
    return inter;
}

template<typename T>
inline constexpr std::set<T> set_difference(const std::set<T>& a,
                                            const std::set<T>& b)
{
    std::set<T> diff;
    std::set_difference(a.begin(),
                        a.end(),
                        b.begin(),
                        b.end(),
                        std::inserter(diff, diff.begin()));
    return diff;
}

#endif  // UTILS_H
