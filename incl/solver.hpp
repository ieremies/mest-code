#ifndef SOLVER_H
#define SOLVER_H

#include "utils.hpp"
#include <gurobi_c++.h>
#include <lemon/list_graph.h>
#include <map>
#include <vector>

class Solver {
  private:
    GRBEnv _env;

  public:
    Solver();
    ~Solver();
    double solve(const Graph &, std::vector<NodeSet> &,
                 std::map<NodeSet, double> &);
};

#endif // SOlVER_H
