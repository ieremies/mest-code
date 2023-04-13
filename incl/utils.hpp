#ifndef UTILS_H
#define UTILS_H

#include <gurobi_c++.h>
#include <lemon/list_graph.h>
#include <set>
#include <vector>

using NodeSet = std::set<lemon::ListGraph::Node>;
using Graph = lemon::ListGraph;
using Color = unsigned int;

class Utils {
  public:
    static void printSolution(const Graph &, const GRBModel &,
                              std::vector<NodeSet>);
    static void printNodeSet(const NodeSet &, const Graph &);
    static void printGraph(const Graph &);
    static void assertColoring(const Graph &, const Graph::NodeMap<Color> &);
};

#endif // UTILS_H
