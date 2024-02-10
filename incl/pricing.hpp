#ifndef PRICING_HPP
#define PRICING_HPP

#include <vector>

#include "utils.hpp"

namespace pricing
{
vector<node_set> solve(const Graph&, const vector<cost>&);
}  // namespace pricing

#endif
