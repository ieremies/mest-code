#ifndef UTILS_H
#define UTILS_H

#include <set>
#include <string>

#include "../incl/graph.hpp"

#define EPS 1e-9
#define TIMELIMIT 3600

using namespace std;
using node_set = set<Graph::node>;
using node = Graph::node;
using edge = Graph::edge;
using color = unsigned int;

string to_string(const node_set&);

#endif  // UTILS_H
