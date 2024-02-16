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
mwis_sol mwis_heu(const branch_node& n, const vector<cost>& weight)
{
    Graph g = Graph(n.g);
    mwis_sol sol = n.sol;

    while (not g.is_empty()) {
        cost max_weight = 0;
        Graph::node max_node = 0;
        for_nodes(g, u) {
            if (weight[u] > max_weight) {
                max_weight = weight[u];
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

cost w(const node_set& s, const vector<cost>& weight)
{
    cost sum = 0;
    for (node const u : s) {
        sum += weight[u];
    }
    return sum;
}

template<typename T>
constexpr std::set<T> set_intersection(const std::set<T>& a,
                                       const std::set<T>& b)
{
    std::set<T> inter;
    std::set_intersection(a.begin(),
                          a.end(),
                          b.begin(),
                          b.end(),
                          std::inserter(inter, inter.begin()));
    return inter;
}

template<typename T>
constexpr std::set<T> set_difference(const std::set<T>& a, const std::set<T>& b)
{
    std::set<T> inter;
    std::set_difference(a.begin(),
                        a.end(),
                        b.begin(),
                        b.end(),
                        std::inserter(inter, inter.begin()));
    return inter;
}
// TODO a porra tem que estarem ordenados

/*
** Function that computes the confining set of a node v.
** Returns an empty set if the node is unconfined.
*/
node_set confine(const Graph& g, Graph::node v, const vector<cost>& weight)
{
    node_set s = {v};

    // while S has an extending child u:
    // add u to S
    while (true) {
        node_set const ons = g.get_open_neighborhood(s);
        node satellite = g.get_n();
        for (node const u : ons) {
            // child : w[u] >= w( S \cap N(u))
            // ext.child : child and
            //             |N(u) \ S| = 1 and
            //             w(u) < w(N(u) \ N(S))
            node_set const onu = g.get_open_neighborhood(u);
            node_set const inter = set_intersection(s, onu);
            if (weight[u] < w(inter, weight)) {
                continue;
            }
            node_set const diff1 = set_difference(onu, s);
            if (diff1.size() != 1) {
                continue;
            }
            node_set const diff2 = set_difference(onu, ons);
            if (weight[u] >= w(diff2, weight)) {
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
        if (weight[u] < w(inter, weight)) {
            continue;
        }

        node_set const diff = set_difference(onu, ons);
        if (weight[u] >= w(diff, weight)) {
            return {};
        }
    }

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
cost mwis_ub(const branch_node n, const vector<cost>& weight)
{
    // sort the vertices in descending order of their weight
    vector<Graph::node> sorted_nodes = {};
    for_nodes(n.g, u) {
        sorted_nodes.push_back(u);
    }
    std::sort(sorted_nodes.begin(),
              sorted_nodes.end(),
              [&n, &weight](Graph::node a, Graph::node b)
              {
                  if (weight[a] == weight[b]) {
                      return n.g.get_degree(a) > n.g.get_degree(b);
                  }
                  return weight[a] > weight[b];
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
            cliques.push_back({{u}, weight[u]});
            wcc += weight[u];
        } else {
            max_clique->first.insert(u);
        }
    }

    return wcc;
}

/*
** Function to apply certain reducion techniques to the graph.
**
** While reducing the graph, it might add some nodes to the current
** solution.
*/
void reduce(branch_node& n, const vector<cost>& weight)
{
    // Xiao2021 rule 1
    // if there is a node v such that w(v) > w(N[v]), then add v to the solution
    // and remove all nodes in N[v] from the graph.
    for_nodes(n.g, v) {
        cost neighbor_sum = 0;
        for_adj(n.g, v, u) {
            neighbor_sum += weight[u];
        }
        if (weight[v] > neighbor_sum) {
            n.sol.value += weight[v];
            n.sol.nodes.insert(v);
            node_set const onu = n.g.get_closed_neighborhood(v);
            for (Graph::node const u : onu) {
                n.g.deactivate(u);
            }
        }
    }

    // TODO Xiao2021 rule 10, degree 1

    // Xiao2021 rule 5
    for_nodes(n.g, v) {
        node_set const conf = confine(n.g, v, weight);
        if (conf.empty()) {
            n.g.deactivate(v);
        }
    }
}

void branch(stack<branch_node>& tree,
            const branch_node& b_node,
            const vector<cost>& weight)
{
    // find the vertex with max degree in G
    Graph::node const v = b_node.g.get_node_max_degree();
    node_set const confining_set = confine(b_node.g, v, weight);

    // Branching 1 : add the confining
    Graph g1 = Graph(b_node.g);
    for (Graph::node const u : g1.get_closed_neighborhood(confining_set)) {
        g1.deactivate(u);
    }

    mwis_sol sol1 = {b_node.sol.value, b_node.sol.nodes};
    for (Graph::node const u : confining_set) {
        sol1.value += weight[u];
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
vector<node_set> pricing::solve(const Graph& orig, const vector<cost>& weight)
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

    stack<branch_node> tree;
    tree.push({g, {0, {}}});
    mwis_sol best = {0, {}};

    while (!tree.empty()) {
        branch_node b_node = tree.top();
        tree.pop();

        // reduce b_node.g and may populate solution b_node.sol
        reduce(b_node, weight);
        if (b_node.g.is_empty()) {
            continue;
        }

        mwis_sol const heu_sol = mwis_heu(b_node, weight);

        // TODO garantir que o EPS aqui é maior que o do gurobi para evitar
        // infinito todo conjunto gerado tem que entrar na base
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

        if (mwis_ub(b_node, weight) <= best.value) {
            continue;
        }

        branch(tree, b_node, weight);
    }

    return new_indep_sets;
}
