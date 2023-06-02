#ifndef PRICING_HPP
#define PRICING_HPP

#include <vector>

#include "utils.hpp"

class Pricing
{
  public:
    static node_set solve(const Graph&, const vector<double>&);
};

#endif
