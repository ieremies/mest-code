#include "../incl/utils.hpp"

string to_string(const node_set& set)
{
    string s = "{";
    for (node u : set) {
        if (s.size() > 1)
            s += ", ";
        s += to_string(u);
    }
    s += "}";
    return s;
}

void log_solution(const Graph& g,
                  const map<node_set, double>& x_s,
                  const double& sol)
{
    vector<node_set> indep_sets;
    for (const auto& [s, x] : x_s) {
        if (x >= 1 - EPS) {
            indep_sets.push_back(s);
        }
    }
    g.apply_changes_to_sol(indep_sets);
    string log = "SOL: %f = ";
    for (const auto& s : indep_sets) {
        log += to_string(s) + " ";
    }
    LOG_F(WARNING, log.c_str(), sol);
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
