#ifndef PRICING_HPP
#define PRICING_HPP

#include "utils.hpp"
#include <vector>

class Pricing {
  public:
    /*
    ** Solve the pricing problem for a given graph and weights of each node.
    ** In this case, we are solving the maximum weighted independent (stable)
    ** set and adding to std::vector<NodeSet> any independent set with weight
    ** greater than 1.
    ** Returns the number of independent sets added.
    */
    static int solve(const Graph &, const Graph::NodeMap<double> &,
                     std::vector<NodeSet> &);
};

#endif
