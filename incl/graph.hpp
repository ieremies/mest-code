#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <bitset>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#define MAX_NODES 10000
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
    node get_active_n() const;
    bool is_empty() const;
    unsigned long int get_m() const;

    node get_degree(node) const;
    node get_node_max_degree() const;
    node get_node_min_degree() const;
    inline bool is_active(node u) const { return active[u]; }

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

    void k_core(int);

    void log() const;
    void apply_changes_to_sol(vector<set<Graph::node>>&) const;

    bool is_connected() const;
    bool is_connected_complement() const;

    // === Weighted functions ================================
    void set_weight(const node&, const double&);
    double get_weight(const node&) const;
    double get_weight(const node_set&) const;

  private:
    node n;
    unsigned long int m;
    vector<mod> delta;
    vector<node> deg;
    bitset<MAX_NODES> active;
    // BUG For some instances, this can be not enough
    vector<vector<node>> adj;
    vector<bitset<MAX_NODES>> adj_bool;
    vector<double> weights;

    void do_conflict(node, node);
    void undo_conflict(node, node);
    void do_contract(node, node);
    void undo_contract(node, node);
    node check_deg(node) const;
    bool check_all_deg() const;
    void complement();
};

using node_set = Graph::node_set;
using node = Graph::node;
using edge = Graph::edge;
#endif
