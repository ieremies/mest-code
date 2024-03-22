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

#include "graph.hpp"
#include "loguru.hpp"

#define EPS 1e-9
#define MAX_GENERATED_SET 100

#define HANDLE_GRB_EXCEPTION(instruction) \
    try { \
        instruction; \
    } catch (GRBException & e) { \
        ABORT_F("GRB_exception = %d (%s)", \
                e.getErrorCode(), \
                e.getMessage().c_str()); \
    }
#define str(x) to_string(x).c_str()

// === Type definitions =======================================================
using namespace std;
using cost = double;
template<typename T>
using matrix = vector<vector<T>>;

struct color_sol  // might be fractional
{
    cost cost = 0.0;
    map<node_set, double> x_sets;

    bool is_integral() const;
    string to_string(const graph&) const;
};

// === Check functions ========================================================
bool check_indep_set(const graph&, const node_set&);
bool check_connectivity(const graph&);
bool check_universal(const graph&);

// === Graph functions ========================================================
void maximal_set(const graph&, node_set&);

// === String functions =======================================================
string to_string(const color_sol&);
string to_string(const node_set&);
inline string to_string(const mod_type& t)
{
    return t == mod_type::contract ? "contract"
        : t == mod_type::conflict  ? "conflict"
                                   : "none";
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
