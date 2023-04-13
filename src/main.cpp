#include "../incl/branch.hpp"
#include "../incl/pricing.hpp"
#include "../incl/solver.hpp"
#include "../incl/utils.hpp"
#include <fstream>
#include <iostream>
#include <lemon/list_graph.h>
#include <string>
#include <vector>

#define EPS 0.0000000001

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

void initialSets(Graph &g, std::vector<NodeSet> &indep_sets) {
    // Trivial independent sets
    for (Graph::NodeIt n(g); n != lemon::INVALID; ++n) {
        NodeSet s;
        s.insert(n);
        indep_sets.push_back(s);
    }
}

int main(int, char **argv) {
    Graph g;
    readDimacsInstance(argv[1], g);

    std::vector<NodeSet> indep_sets;
    initialSets(g, indep_sets);

    // Branch tree;
    // tree.addNode(g, indep_sets);

    // while (true) {
    //     // BUG o correto não seria "pega o próximo a dar branch?"
    //     g, indep_sets = Branch::next();
    while (true) {
        Graph::NodeMap<double> weight(g);
        Solver::solve(g, indep_sets, weight);
        // Trocar pra o pricing add constrain
        if (Pricing::solve(g, weight, indep_sets) == 0)
            break;
        // cleanIndepSets(g, weight, indep_sets);
    }
    //     // espero que Branch::branch retorne a quantidade de nós gerados
    //     if (Branch::branch(g, weight, indep_sets) == 0)
    //         break;
    // }
    return 0;
}
