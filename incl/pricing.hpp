#ifndef PRICING_HPP
#define PRICING_HPP

#include "utils.hpp"
#include <gurobi_c++.h>
#include <vector>

class Pricing {
  public:
    static NodeSet solve(const Graph &, const Graph::NodeMap<double> &);
};

#endif
