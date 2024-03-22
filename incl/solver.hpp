#ifndef SOLVER_H
#define SOLVER_H

#include <map>
#include <vector>

#include <gurobi_c++.h>

#include "formulation.hpp"
#include "utils.hpp"

class environment
{
  public:
    static GRBEnv& get_env()
    {
        static GRBEnv env;
        // disable gurobi license output
        env.set(GRB_DoubleParam_FeasibilityTol, EPS);
        env.set(GRB_DoubleParam_OptimalityTol, EPS);
        env.set(GRB_IntParam_LogToConsole, 0);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.set(GRB_IntParam_NumericFocus, 1);
        // make gurobi use only one thread
        if (SINGLE_THREAD) {
            env.set(GRB_IntParam_Threads, 1);
        }
        env.start();
        return env;
    }
};

class solver
{
    GRBEnv env;
    formulation form;
    GRBModel grb;
    map<node_set, GRBVar> vars;
    vector<GRBConstr> constrs;

  public:
    explicit solver(formulation&);
    color_sol solve();
    void add_variable(const node_set&);
    void add_constrain(const node_set&);
    formulation& get_formulation() { return form; }
};

#endif  // SOlVER_H
