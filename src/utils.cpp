#include "../incl/utils.hpp"

#include "../lib/loguru.hpp"

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
    string log = "New upper bound with value: %f = ";
    for (auto& [s, x] : x_s) {
        if (x >= 1 - EPS) {
            log += to_string(s) + " ";
        }
    }
    LOG_F(WARNING, log.c_str(), sol);
    g.log_changes();
}
