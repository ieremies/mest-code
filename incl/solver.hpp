#ifndef SOLVER_H
#define SOLVER_H

#include "utils.hpp"
#include <gurobi_c++.h>
#include <lemon/list_graph.h>
#include <vector>

class Solver {
  private:
    GRBEnv _env;

  public:
    Solver();
    ~Solver();
    double solve(const Graph &, std::vector<NodeSet> &, std::vector<double> &);
};

#endif // SOlVER_H
