#ifndef SOLVER_H
#define SOLVER_H

#include "utils.hpp"
#include <gurobi_c++.h>
#include <lemon/list_graph.h>
#include <vector>

class Solver {
  private:
    static GRBEnv _env;

  public:
    static void solve(const Graph &, const std::vector<NodeSet> &,
                      Graph::NodeMap<double> &);
};

#endif // SOlVER_H
