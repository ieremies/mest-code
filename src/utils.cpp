#include "../incl/utils.hpp"
#include <cassert>

void Utils::printSolution(const Graph &g, const GRBModel &model,
                          const std::vector<NodeSet> indep_sets) {
    for (int i = 0; i < model.get(GRB_IntAttr_NumVars); i++) {
        if (model.getVar(i).get(GRB_DoubleAttr_X) > 0.5) {
            std::cout << "Set " << i << " selected" << std::endl;
            for (auto n : indep_sets[i]) {
                std::cout << g.id(n) << " ";
            }
            std::cout << std::endl;
        }
    }
}

/* check if a coloring is valid
 * i.e. no two adjacent nodes have the same color
 * returns true if the coloring is valid, false otherwise
 * BUG does not compile becouse of assert
 */
// void assertColoring(const Graph &g, const Graph::NodeMap<Color> s) {
//     for (lemon::ListGraph::EdgeIt e(g); e != lemon::INVALID; ++e) {
//         lemon::ListGraph::Node u = g.u(e);
//         lemon::ListGraph::Node v = g.v(e);
//         assert(s[u] != s[v]);
//     }
// }

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
