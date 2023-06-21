#ifndef SOLVER_H
#define SOLVER_H

#include <map>
#include <vector>

#include <gurobi_c++.h>

#include "utils.hpp"

class Solver
{
  private:
    GRBEnv _env;
    Solver();

  public:
    double solve(const Graph&, vector<node_set>&, map<node_set, double>&);
    static Solver& get_instance()
    {
        static Solver instance;  // Singleton instance
        return instance;
    }
};

#endif  // SOlVER_H
