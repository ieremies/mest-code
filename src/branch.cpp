#include "../incl/branch.hpp"
#include "../incl/utils.hpp"
#include "../lib/loguru.hpp"
#include <lemon/core.h>
#include <map>

// Construtor de BranchNode para aqueles ainda não resolvidos
BranchNode::BranchNode(const Graph &origin_g,
                       const std::vector<NodeSet> &other_indep_sets,
                       Graph::NodeMap<Graph::Node> &nr)
    : indep_sets(other_indep_sets) {
    lemon::graphCopy(origin_g, g).nodeRef(nr).run();
}

BranchNode::~BranchNode() {}

typedef std::pair<Graph::Node, Graph::Node> NodePair;

bool find_vertexes(const Graph &g, const std::vector<NodeSet> &indep_set,
                   std::map<NodeSet, double> x_s, Graph::Node &u,
                   Graph::Node &v) {
    int nnodes = lemon::countNodes(g);
    std::map<NodePair, double> diff;
    for (NodeSet set : indep_set)
        for (Graph::Node u : set)
            for (Graph::Node v : set) {
                if (u == v or g.id(u) > g.id(v))
                    continue;
                // if (u,v) is in map, just add the value
                if (diff.find(std::make_pair(u, v)) != diff.end())
                    diff[std::make_pair(u, v)] += x_s[set];
                else
                    diff.insert(std::make_pair(std::make_pair(u, v), x_s[set]));
            }
    // iterate over all pairs in the map diff and find the closest to 0.5
    double min = 100;
    for (auto it = diff.begin(); it != diff.end(); it++) {
        if (std::abs(0.5 - it->second) < min) {
            min = std::abs(0.5 - it->second);
            u = it->first.first;
            v = it->first.second;
        }
    }

    if (min >= 0.5)
        return false;
    return true;
}

// ========================= Branch =========================
Branch::Branch(const Graph &g, const std::vector<NodeSet> &indep_set) {
    tree = std::stack<BranchNode *>();
    Graph::NodeMap<Graph::Node> nr(g);
    BranchNode *node = new BranchNode(g, indep_set, nr);
    tree.push(node);
}

Branch::~Branch() {
    while (!tree.empty()) {
        BranchNode *node = tree.top();
        tree.pop();
        delete node;
    }
}

void clean_indep_sets_conflict(std::vector<NodeSet> &indep_sets,
                               const Graph::Node &u, const Graph::Node &v) {
    NodeSet *to_remove = NULL;
    for (NodeSet set : indep_sets) {
        if (set.find(u) != set.end() and set.find(v) != set.end()) {
            to_remove = &set;
            break;
        }
    }
    if (to_remove != NULL) {
        indep_sets.erase(
            std::remove(indep_sets.begin(), indep_sets.end(), *to_remove),
            indep_sets.end());
    }
}

void clean_indep_sets_join(std::vector<NodeSet> &indep_sets,
                           const Graph::Node &u, const Graph::Node &v) {
    for (NodeSet set : indep_sets) {
        if (set.find(v) != set.end() or set.find(u) != set.end()) {
            std::remove(indep_sets.begin(), indep_sets.end(), set),
                indep_sets.end();
        }
    }
}

int Branch::branch(const Graph &g, const std::vector<NodeSet> &indep_sets,
                   const std::map<NodeSet, double> &x_s) {
    // Find two nodes to branch the tree based on then.
    // If none could be found, we donnot branch.
    Graph::Node u, v;
    if (not find_vertexes(g, indep_sets, x_s, u, v))
        return 0;
    LOG_F(INFO, "Branching on %d and %d", g.id(u), g.id(v));

    // One branch has u and v in different independent sets
    // TODO alterar o indep_sets para refletir as mudanças
    Graph::NodeMap<Graph::Node> nr1(g);
    BranchNode *node1 = new BranchNode(g, indep_sets, nr1);
    node1->g.addEdge(nr1[u], nr1[v]);
    tree.push(node1);

    Graph::NodeMap<Graph::Node> nr2(g);
    BranchNode *node2 = new BranchNode(g, indep_sets, nr2);
    node2->g.contract(nr2[u], nr2[v]); // u permenece, v é deletado
    tree.push(node2);

    return 2; // the number of branches created
}

void Branch::next(Graph &g, std::vector<NodeSet> &indep_sets) {
    LOG_SCOPE_F(INFO, "Next");
    BranchNode *node = tree.top();
    tree.pop();
    lemon::graphCopy(node->g, g).run();
    indep_sets = node->indep_sets;
    delete node;

    LOG_F(INFO, "Branching on %d vertexes and %d edges", lemon::countNodes(g),
          lemon::countEdges(g));
}
