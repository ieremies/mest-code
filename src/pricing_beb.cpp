#include <stack>

#include "pricing.hpp"

struct mwis_sol
{
    double value;
    node_set nodes;
};

struct branch_node
{
    Graph g;
    mwis_sol sol;
};

/*
** Function to apply certain reducion techniques to the graph.
**
** While reducing the graph, it might add some nodes to the current
** solution.
*/
void reduce(branch_node n, const vector<double>& weight)
{
    // Xiao2021 rule 1
    // if there is a node v such that w(v) > w(N[v]), then add v to the solution
    // and remove all nodes in N[v] from the graph.
    for_nodes(n.g, v) {
        double neighbor_sum = 0;
        for_adj(n.g, v, u) {
            neighbor_sum += weight[u];
        }
        if (weight[v] > neighbor_sum) {
            LOG_F(INFO, "Rule 1: Adding node %d to the solution.", v);
            n.sol.value += weight[v];
            n.sol.nodes.insert(v);
            for_adj(n.g, v, u) {
                n.g.deactivate(u);
            }
        }
    }

    // Xiao2021 rule 10, degree 1
    // let u be the neighbor of v, if deg(v) = 1,
    // if w(u) <= w(v), remove u.
    // else w(u) := w(u) - w(v) and remove v.
    // TODO
}

/*
** Heuristic to, given the current solution and graph, find a solution MWIS.
*/
mwis_sol mwis_heu(branch_node n, const vector<double>& weight)
{
    Graph g = Graph(n.g);
    mwis_sol sol = n.sol;

    double max_weight = 0;
    Graph::node max_node = 0;
    for_nodes(g, u) {
        if (weight[u] > max_weight) {
            max_weight = weight[u];
            max_node = u;
        }
    }

    sol.value += max_weight;
    sol.nodes.insert(max_node);
    g.deactivate(max_node);
    for_adj(g, max_node, u) {
        g.deactivate(u);
    }

    return sol;
}

/*
** Function that computes the confining set of a node v.
** Returns an empty set if the node is unconfined.
*/
node_set confine(const Graph& /*g*/,
                 Graph::node v,
                 const vector<double>& /*weight*/)
{
    node_set confining_set = {v};

    // while S has an extending child u:
    // w[u] >= w( S \cap N(u)) and
    // |N(u) \ S| = 1 and
    // w(u) < w(N(u) \ N(S))
    // add u to S
    return {};
}

vector<node_set> pricing::solve(const Graph& orig, const vector<double>& weight)
{
    LOG_SCOPE_F(INFO, "Pricing.");
    Graph g = Graph(orig);
    vector<node_set> new_indep_sets = {};

    // Remove all nodes with weight 0
    for_nodes(g, n) {
        if (weight[n] < 0 + EPS) {
            g.deactivate(n);
        }
    }
    g.log();

    /*
    ** while there is a branch-and-bound node to be searched
    ** get the next one
    ** G, sol <- reduce(G, w)
    ** actual = sol + greedy(G, w)
    ** best <- max{best, actual}
    ** if actual <= best, then continue
    ** v <- the vertex with max degree in G
    ** add node (G - N[Sv], sol + v) to the branch-and-bound tree
    ** add node (G - v, sol) to the branch-and-bound tree
    */
    stack<branch_node> tree;
    tree.push({g, {0, {}}});
    mwis_sol best = {0, {}};

    while (!tree.empty()) {
        LOG_F(INFO, "Tree size: %zu", tree.size());
        LOG_F(INFO, "Best value: %f", best.value);
        branch_node const b_node = tree.top();
        tree.pop();

        reduce(b_node, weight);
        b_node.g.log();
        mwis_sol const actual = mwis_heu(b_node, weight);

        if (actual.value > 1 + EPS) {
            new_indep_sets.push_back(actual.nodes);
        }

        if (actual.value <= best.value) {
            continue;
        }
        best = actual;

        // find the vertex with max degree in G
        Graph::node const v = b_node.g.get_node_max_degree();
        node_set const confining_set = confine(b_node.g, v, weight);

        // g1 <- G - N[Sv]
        Graph g1 = Graph(b_node.g);
        for (auto u : confining_set) {
            for_adj(g1, u, n) {
                g1.deactivate(n);
            }
        }
        mwis_sol sol1 = {actual.value, actual.nodes};
        sol1.value += weight[v];
        sol1.nodes.insert(v);
        if (not g1.is_empty()) {
            tree.push({g1, sol1});
        }

        // g2 <- G - v
        Graph g2 = Graph(b_node.g);
        g2.deactivate(v);
        if (not g2.is_empty()) {
            tree.push({g2, actual});
        }
        exit(1);
    }

    return new_indep_sets;
}
