#include "../incl/branch.hpp"
#include <lemon/core.h>

int Branch::branch(Graph &g, Graph::NodeMap<double> &weight,
                   std::vector<NodeSet> &indep_sets) {
    // Todos os indep_sets devem ter soma <= 0+eps ou >= 1-eps;
    NodeSet chosen;
    for (auto set : indep_sets) {
        double sum;
        for (auto node : set)
            sum += weight[node];
    }
    // Se esse for o caso para todos, show.
    // Caso contrário, criamos dois nós na árvore de branch
    // Determinamos dois nós u e v
    Graph::Node u, v;
    int degree_u, degree_v;
    for (auto node : chosen) {
        if (degree_u < g.degree(node)) {
            degree_v = degree_u;
            v = u;
            degree_u = g.degree(node);
            u = node;
        } else if (degree_v < g.degree(node)) {
            degree_v = g.degree(node);
            v = node;
        }
    }

    // Um com uma aresta adicionada entre u e v
    BranchNode *node1 = new BranchNode();
    lemon::graphCopy(g, node1->g);
    node1->g.addEdge(u, v);
    this->tree.push(node1);
    // E outro onde u e v são o mesmo vértice
    BranchNode *node2 = new BranchNode();
    lemon::graphCopy(g, node2->g);
    node2->g.contract(u, v);
    this->tree.push(node2);
}
