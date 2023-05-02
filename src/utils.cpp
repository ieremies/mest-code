#include "../incl/utils.hpp"
#include <cassert>

void Utils::printSolution(const Graph &g,
                          const std::vector<NodeSet> &indep_sets,
                          const Graph::NodeMap<double> &weight) {
    for (auto &set : indep_sets) {
        double sum = 0;
        for (auto &node : set) {
            std::cout << g.id(node) + 1 << " ";
            sum += weight[node];
        }
        std::cout << " -> " << sum << std::endl;
    }
}

/* Function to print a graph */
void Utils::printGraph(const lemon::ListGraph &g) {
    // Print the number of nodes and edges
    std::cout << "Number of nodes: " << countNodes(g) << std::endl;
    std::cout << "Number of edges: " << countEdges(g) << std::endl;

    // Iterate over the nodes and print their IDs and degrees
    for (lemon::ListGraph::NodeIt n(g); n != lemon::INVALID; ++n) {
        std::cout << "Node " << g.id(n) << " has degree " << countIncEdges(g, n)
                  << std::endl;
    }

    // Iterate over the edges and print their endpoints
    for (lemon::ListGraph::EdgeIt e(g); e != lemon::INVALID; ++e) {
        std::cout << "Edge " << g.id(e) << " connects nodes " << g.id(g.u(e))
                  << " and " << g.id(g.v(e)) << std::endl;
    }
}

void Utils::printNodeSet(const NodeSet &s, const Graph &g) {
    std::cout << "New set: ";
    for (auto n : s) {
        std::cout << g.id(n) << " ";
    }
    std::cout << std::endl;
}
//
bool checkInteger(Graph::NodeMap<double> &weights,
                  std::vector<NodeSet> &indep_sets) {
    for (auto &set : indep_sets) {
        double sum = 0;
        for (auto &node : set) {
            sum += weights[node];
        }
        if (sum < 1 - EPS or sum > EPS) {
            return false;
        }
    }
    return true;
}

std::string Utils::NodeSet_to_string(const Graph &g, const NodeSet &set) {
    std::string s = "{ ";
    for (Graph::Node n : set) {
        s += std::to_string(g.id(n)) + ", ";
    }
    s += "}";
    return s;
}
