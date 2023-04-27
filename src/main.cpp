#include "../incl/branch.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
** Function to read a DIMACS instanc from file (filename) and
** create the Graph g.
*/
void readDimacsInstance(std::string filename, Graph &g) {
    std::ifstream infile(filename);
    std::string line;
    int n_vertices, m_edges;

    do {
        getline(infile, line);
    } while (line[0] != 'p');

    // format line
    sscanf(line.c_str(), "p edge %d %d", &n_vertices, &m_edges);

    std::vector<Graph::Node> nodes(n_vertices);
    for (int i = 0; i < n_vertices; i++)
        nodes[i] = g.addNode();

    // add edges to the graph
    for (int i = 0; i < m_edges; i++) {
        int u, v;
        getline(infile, line);
        sscanf(line.c_str(), "e %d %d", &u, &v);
        g.addEdge(nodes[u - 1], nodes[v - 1]);
    }
}

/*
** Defines the initial independent sets of g and place them in
** vector<NodeSet> indep_sets.
*/
void initialSets(Graph &g, std::vector<NodeSet> &indep_sets) {
    // Trivial independent sets
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n) {
        NodeSet s;
        s.insert(n);
        indep_sets.push_back(s);
    }
}

/*
** Defines the initial upper bound.
*/
int primal_heuristic(const Graph &g) { return 10000000; }

int main(int, char **argv) {
    Graph g;
    readDimacsInstance(argv[1], g);

    int upper_bound = primal_heuristic(g);

    std::vector<NodeSet> indep_sets;
    initialSets(g, indep_sets);

    double sol = 0; // place holder

    // Create the branching tree with the initial node
    Branch tree(g, indep_sets);
    while (true) {
        tree.next(g, indep_sets);
        indep_sets.clear();
        initialSets(g, indep_sets);

        std::vector<double> x_s(0, 0);
        Solver solver = Solver();
        sol = solver.solve(g, indep_sets, x_s); // BUG esse sol é obj da dual
        std::cout << "sol: " << sol << std::endl;

        if (tree.branch(g, indep_sets, x_s) == 0)
            break;
    }

    return 0;
}
