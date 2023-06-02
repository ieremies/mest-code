#ifndef SOLVER_H
#define SOLVER_H

#include <map>
#include <vector>

#include <gurobi_c++.h>

#include "utils.hpp"

class Solver
{
    GRBEnv _env;

  public:
    Solver();
    double solve(const Graph&, vector<node_set>&, map<node_set, double>&);
};

#endif  // SOlVER_H
