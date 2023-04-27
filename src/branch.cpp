#include "../incl/branch.hpp"
#include "../incl/utils.hpp"
#include <lemon/core.h>

// Construtor de BranchNode para aqueles ainda não resolvidos
BranchNode::BranchNode(const Graph &origin_g,
                       const std::vector<NodeSet> &other_indep_sets)
    : indep_sets(other_indep_sets) {
    lemon::graphCopy(origin_g, g).run();
}

BranchNode::~BranchNode() {}

int find_vertexes(const Graph &g, const std::vector<NodeSet> &indep_set,
                  const std::vector<double> x_s, Graph::Node &u,
                  Graph::Node &v) {
    int nnodes = lemon::countNodes(g);
    std::vector<std::vector<double>> diff(nnodes,
                                          std::vector<double>(nnodes, 0));
    for (int i = 0; i < indep_set.size(); i++)
        for (auto u : indep_set[i])
            for (auto v : indep_set[i])
                diff[g.id(u)][g.id(v)] += x_s[i];

    // find the value in diff that is closest to 0.5
    double min = 10000000;
    for (int i = 0; i < nnodes; i++)
        for (int j = 0; j < nnodes; j++)
            if (std::abs(diff[i][j] - 0.5) < min) {
                min = diff[i][j];
                u = g.nodeFromId(i);
                v = g.nodeFromId(j);
            }

    if (min != 10000000)
        return 1;
    return 0;
}

// ========================= Branch =========================
Branch::Branch(const Graph &g, const std::vector<NodeSet> &indep_set) {
    tree = std::stack<BranchNode *>();
    BranchNode *node = new BranchNode(g, indep_set);
    tree.push(node);
}

Branch::~Branch() {
    while (!tree.empty()) {
        BranchNode *node = tree.top();
        tree.pop();
        delete node;
    }
}

int Branch::branch(const Graph &g, const std::vector<NodeSet> &indep_sets,
                   const std::vector<double> &x_s) {
    // Find two nodes to branch the tree based on then.
    // If none could be found, we donnot branch.
    Graph::Node u, v;
    if (find_vertexes(g, indep_sets, x_s, u, v) == 0)
        return 0;

    // One branch has u and v in different independent sets
    // TODO alterar o indep_sets para refletir as mudanças
    BranchNode *node1 = new BranchNode(g, indep_sets);
    node1->g.addEdge(u, v);
    tree.push(node1);

    // The othr has u and v in the same independent set
    BranchNode *node2 = new BranchNode(g, indep_sets);
    node2->g.contract(u, v); // u permenece, v é deletado
    tree.push(node2);

    return 2; // the number of branches created
}

void Branch::next(Graph &g, std::vector<NodeSet> &indep_sets) {
    BranchNode *node = tree.top();
    tree.pop();
    lemon::graphCopy(node->g, g).run();
    indep_sets = node->indep_sets;
    delete node;
}
