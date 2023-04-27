#ifndef PRICING_HPP
#define PRICING_HPP

#include "utils.hpp"
#include <gurobi_c++.h>
#include <vector>

class Pricing {
  public:
    /*
    ** Solve the pricing problem for the given graph and its dual solution
    ** As thre result, we add the respective constrains to the model and
    ** to the vector of independent sets.
    */
    static bool solve(const Graph &, const Graph::NodeMap<double> &,
                      std::vector<NodeSet> &, GRBModel &,
                      const Graph::NodeMap<GRBVar> &, std::vector<GRBConstr> &);
};

#endif
