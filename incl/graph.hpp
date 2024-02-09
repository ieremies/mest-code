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
    using node_set = set<node>;
    using edge = pair<node, node>;

    struct mod
    {
        mod_type t;
        node u, v;
    };

    // === Constructors ======================================
    explicit Graph(int);
    Graph(const Graph&);

    // === Getters ===========================================
    node get_n() const;  // number of active nodes
    bool is_empty() const;
    unsigned long int get_m() const;  // FIXME this function is not clear

    node get_degree(node) const;
    node get_node_max_degree() const;
    bool is_active(node) const;

    /*
     * @brief Get the number of edges beteween two nodes. If either are
     * inactive or equal, their adjacency is 0.
     */
    node get_adjacency(node, node) const;

    // === Used for iterators ================================
    node first_act_node() const;
    node next_act_node(node) const;

    node first_adj_node(node) const;
    node next_adj_node(node, node) const;

    edge first_edge() const;
    edge next_edge(edge) const;

    node_set get_open_neighborhood(const node&) const;
    node_set get_open_neighborhood(const node_set&) const;
    node_set get_closed_neighborhood(const node&) const;
    node_set get_closed_neighborhood(const node_set&) const;

    // === Functions to modify the graph =====================
    void deactivate(node);

    void change(mod_type, node, node);
    void undo(mod_type, node, node);

    unsigned long int add_edge(node, node);
    unsigned long int remove_edge(node, node);

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
    bool check_all_deg() const;
};

using node_set = Graph::node_set;
using node = Graph::node;
using edge = Graph::edge;
#endif
