#include <algorithm>
#include <stack>

#include "pricing.hpp"

#include "../incl/utils.hpp"

struct mwis_sol
{
    cost value;
    node_set nodes;
};

struct branch_node
{
    Graph g;
    mwis_sol sol;
};

/*
** Heuristic to, given the current solution and graph, find a solution MWIS.
*/
mwis_sol mwis_heu(const branch_node& n)
{
    Graph g = Graph(n.g);
    mwis_sol sol = n.sol;

    while (not g.is_empty()) {
        cost max_weight = 0;
        Graph::node max_node = 0;
        for_nodes(g, u) {
            if (n.g.get_weight(u) > max_weight) {
                max_weight = n.g.get_weight(u);
                max_node = u;
            }
        }

        sol.value += max_weight;
        sol.nodes.insert(max_node);
        node_set const onu = g.get_closed_neighborhood(max_node);
        for (Graph::node const u : onu) {
            g.deactivate(u);
        }
    }

    return sol;
}

/*
** Function that computes the confining set of a node v.
** Returns an empty set if the node is unconfined.
*/
node_set confine(const Graph& g, Graph::node v)
{
    // TODO Refazer isso com o novo algoritmo de Xiao2023
    node_set s = {v};

    // while S has an extending child u:
    // add u to S
    while (true) {
        node_set const ons = g.get_open_neighborhood(s);
        node satellite = g.get_n();
        for (node const u : ons) {
            // child : w[u] >= w( S \cap N(u))
            node_set const onu = g.get_open_neighborhood(u);
            node_set const inter = set_intersection(s, onu);
            if (g.get_weight(u) < g.get_weight(inter)) {
                continue;
            }
            // ext.child : child and
            //             |N(u) \ S| = 1 and
            //             w(u) < w(N(u) \ N(S))
            node_set const diff1 = set_difference(onu, s);
            if (diff1.size() != 1) {
                continue;
            }
            node_set const diff2 = set_difference(onu, ons);
            if (g.get_weight(u) >= g.get_weight(diff2)) {
                continue;
            }
            satellite = *diff1.begin();
            break;
        }
        if (satellite == g.get_n()) {
            break;
        }
        s.insert(satellite);
    }

    // if there is a child u of S such that w(u) >= w(N(u) \ S), then
    // return {}
    node_set const ons = g.get_open_neighborhood(s);
    for (node const u : ons) {
        // if it is not child, continue
        node_set const onu = g.get_open_neighborhood(u);
        node_set const inter = set_intersection(s, onu);
        if (g.get_weight(u) < g.get_weight(inter)) {
            continue;
        }

        node_set const diff = set_difference(onu, ons);
        if (g.get_weight(u) >= g.get_weight(diff)) {
            return {};
        }
    }

    DCHECK_F(check_indep_set(g, s), "Confining set is not independent.");

    return s;
}

/*
** Xiao2021 indicates the algorithm of Lamm2018 for Weighted Clique Cover.
** Lamm2018 notes that this algorithm produces a higher WCC than the
** method of Brelaz (see Brelaz1976 and Akiba2016)
**
** "We begin by sorting the vertices in descending order of their weight
** (ties are broken by selecting the vertex with higher degree). Next,
** we initiate an empty set of cliques C. We then iterate over the sorted
** vertices and search for the clique with maximum weight which it can
** be added to. If there are no candidates for insertion, we insert a new
** single vertex clique to C and assign it the weight of the vertex.
** Afterwards the vertex is marked as processed and we continue with the
** next one." -- Lamm2018, page6
*/
cost mwis_ub(const branch_node n)
{
    // sort the vertices in descending order of their weight
    vector<Graph::node> sorted_nodes = {};
    for_nodes(n.g, u) {
        sorted_nodes.push_back(u);
    }
    std::sort(sorted_nodes.begin(),
              sorted_nodes.end(),
              [&n](Graph::node a, Graph::node b)
              {
                  if (n.g.get_weight(a) == n.g.get_weight(b)) {
                      return n.g.get_degree(a) > n.g.get_degree(b);
                  }
                  return n.g.get_weight(a) > n.g.get_weight(b);
              });

    // iterate over the sorted vertices and search for the clique with
    // maximum weight which it can be added to.
    vector<pair<node_set, cost>> cliques = {};
    cost wcc = 0;
    for (Graph::node const u : sorted_nodes) {
        pair<node_set, cost>* max_clique = nullptr;
        node_set const onu = n.g.get_open_neighborhood(u);
        for (pair<node_set, cost>& clique : cliques) {
            if (clique.first == onu
                and (max_clique == nullptr
                     or clique.second > max_clique->second))
            {
                max_clique = &clique;
            }
        }
        if (max_clique == nullptr) {
            cliques.push_back({{u}, n.g.get_weight(u)});
            wcc += n.g.get_weight(u);
        } else {
            max_clique->first.insert(u);
        }
    }

    return wcc;
}

/*
** Xiao2021 rule 1
** if there is a node v such that w(v) > w(N[v]), then add v to the solution
** and remove all nodes in N[v] from the graph.
**/
void xiao2021_rule1(branch_node& n)
{
    for_nodes(n.g, v) {
        cost neighbor_sum = 0;
        for_adj(n.g, v, u) {
            neighbor_sum += n.g.get_weight(u);
        }
        if (n.g.get_weight(v) < neighbor_sum) {
            continue;
        }
        n.sol.value += n.g.get_weight(v);
        n.sol.nodes.insert(v);
        node_set const onu = n.g.get_closed_neighborhood(v);
        for (Graph::node const u : onu) {
            n.g.deactivate(u);
        }
    }
}

/*
** Xiao2021 rule 5
** If a vertex is unconfinaded, remove it from the graph.
*/
void xiao2021_rule5(branch_node& n)
{
    for_nodes(n.g, v) {
        node_set const conf = confine(n.g, v);
        if (conf.empty()) {
            n.g.deactivate(v);
        }
    }
}

/*
** For every degree 1 node (v):
** remove its neighbor (u) if w(u) <= w(v)
** otherwise, update w(u) := w(u) - w(v) and remove v
*/
void xaiao2021_rule10_deg1(branch_node& n) {}

/*
** Function to apply certain reducion techniques to the graph.
**
** While reducing the graph, it might add some nodes to the current
** solution.
*/
void reduce(branch_node& n)
{
    xiao2021_rule1(n);
    xiao2021_rule5(n);
}

/*
** Function that determines wheter or not to branch.
** If so, add the branch to the "tree" (stack).
*/
void branch(stack<branch_node>& tree, branch_node& b_node)
{
    // find the vertex with max degree in G, if it is not confined, remove it.
    node_set confining_set;
    Graph::node v = 0;
    while (true) {
        v = b_node.g.get_node_max_degree();
        confining_set = confine(b_node.g, v);
        if (not confining_set.empty()) {
            break;
        }
        b_node.g.deactivate(v);
    }

    // Branching 1 : add the confining
    Graph g1 = Graph(b_node.g);
    for (Graph::node const u : g1.get_closed_neighborhood(confining_set)) {
        g1.deactivate(u);
    }

    mwis_sol sol1 = {b_node.sol.value, b_node.sol.nodes};
    for (Graph::node const u : confining_set) {
        sol1.value += b_node.g.get_weight(u);
        sol1.nodes.insert(u);
    }

    if (not g1.is_empty()) {
        tree.push({g1, sol1});
    }

    // Branching 2 : delete v
    Graph g2 = Graph(b_node.g);
    g2.deactivate(v);
    if (not g2.is_empty()) {
        tree.push({g2, b_node.sol});
    }
}

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
vector<node_set> pricing::solve(const Graph& orig)
{
    LOG_SCOPE_F(INFO, "Pricing.");
    Graph g = Graph(orig);
    vector<node_set> new_indep_sets = {};

    // Remove all nodes with weight 0
    for_nodes(g, n) {
        if (g.get_weight(n) <= 0) {
            g.deactivate(n);
        }
    }

    log_graph_stats(g, "Original");
    log_graph_stats(g, "Reduced");

    stack<branch_node> tree;
    tree.push({g, {0, {}}});
    mwis_sol best = {0, {}};

    int count = 0;
    while (!tree.empty()) {
        count++;
        branch_node b_node = tree.top();
        tree.pop();

        // reduce b_node.g and may populate solution b_node.sol
        reduce(b_node);

        // TODO Xiao2023 says we can use some algorithm when the graph is small
        // to quickly find the MWIS.

        mwis_sol const heu_sol = mwis_heu(b_node);

        // BUG Caso infinito, conferir se EPS é maior que o EPS dado ao Gurobi.
        if (heu_sol.value > 1 + EPS) {
            // check if is not aldready in new_indep_set
            if (find(
                    new_indep_sets.begin(), new_indep_sets.end(), heu_sol.nodes)
                != new_indep_sets.end())
            {
                continue;
            }
            new_indep_sets.push_back(heu_sol.nodes);
        }

        if (heu_sol.value > best.value) {
            best = heu_sol;
        }
        if (b_node.g.is_empty()) {
            continue;
        }
        if (mwis_ub(b_node) <= best.value) {
            continue;
        }

        branch(tree, b_node);
    }
    LOG_F(INFO, "MWIS solved with value %Lf | %d branchs.", best.value, count);

    LOG_SCOPE_F(INFO, "Maximal set.");
    for (node_set& s : new_indep_sets) {
        maximal_set(g, s);
    }

    LOG_F(INFO, "Found %lu independent sets violated.", new_indep_sets.size());

    return new_indep_sets;
}
