#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#define for_nodes(G, u) \
    for (node u = G.first_act_node(); u < G.get_n(); u = G.next_act_node(u))
#define for_edges(G, e) \
    for (edge e = G.first_edge(); \
         e.first < G.get_n() and e.second < G.get_n(); \
         e = G.next_edge(e))
#define for_adj(G, u, n) \
    for (node n = G.first_adj_node(u); n < G.get_n(); n = G.next_adj_node(u, n))

using namespace std;

enum class mod_type
{
    conflict,  // add an edge between u and v
    contract  // all edges of v are added to u and v is deactivated
};

class Graph
{
  public:
    using node = short unsigned int;
    using edge = pair<node, node>;

    struct mod
    {
        mod_type t;
        node u, v;
    };

    explicit Graph(int);
    explicit Graph(string filename);

    // copy constructor
    Graph(const Graph&);

    void deactivate(node);
    void activate(node);

    bool is_empty() const;

    node get_n() const;
    unsigned long int get_m() const;

    void change(mod_type, node, node);
    void undo(mod_type, node, node);

    node get_degree(node) const;
    node max_degree() const;
    bool is_active(node) const;

    unsigned long int add_edge(node, node);
    unsigned long int remove_edge(node, node);
    node get_adjacency(node, node) const;

    node first_act_node() const;
    node next_act_node(node) const;

    node first_adj_node(node) const;
    node next_adj_node(node, node) const;

    edge first_edge() const;
    edge next_edge(edge) const;

    void log() const;
    void apply_changes_to_sol(vector<set<Graph::node>>&) const;

  private:
    node n;
    unsigned long int m;
    vector<mod> delta;
    vector<node> deg;
    vector<bool> active;
    // BUG For some instances, this can be not enough
    vector<vector<node>> adj;

    void do_conflict(node, node);
    void undo_conflict(node, node);
    void do_contract(node, node);
    void undo_contract(node, node);
    node check_deg(node) const;
};

using node_set = set<Graph::node>;
using node = Graph::node;
using edge = Graph::edge;
#endif
