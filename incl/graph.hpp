#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <bitset>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using std::bitset;
using std::pair;
using std::set;
using std::string;
using std::vector;

enum
{
    max_nodes = 10000
};
#define for_nodes(G, u) \
    for (node u = (G).first_act_node(); (u) < (G).get_n(); \
         (u) = (G).next_act_node(u))
#define for_edges(G, e) \
    for (edge e = (G).first_edge(); \
         (e).first < (G).get_n() and (e).second < (G).get_n(); \
         (e) = (G).next_edge(e))
#define for_adj(G, u, n) \
    for (node n = (G).first_adj_node(u); (n) < (G).get_n(); \
         (n) = (G).next_adj_node(u, n))
#define for_pair_nodes(G, u, v) \
    for (node u = (G).first_act_node(); (u) < (G).get_n(); \
         (u) = (G).next_act_node(u)) \
        for (node v = (G).next_act_node(u); (v) < (G).get_n(); \
             (v) = (G).next_act_node(v))

enum class mod_type
{
    // none -> contract -> conflict
    none,  // nothing has been done
    contract,  // all edges of v are added to u and v is deactivated
    conflict,  // add an edge between u and v
};

class graph
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
    explicit graph(int);
    explicit graph(const string&);
    graph(const graph& g);
    ~graph() = default;

    // === Getters ===========================================
    node get_n() const;
    node get_active_n() const;
    bool is_empty() const;
    unsigned long int get_m() const;

    node get_degree(node) const;
    node get_node_max_degree() const;
    node get_node_min_degree() const;
    inline unsigned int get_n_mods() const { return delta.size(); }
    inline bool is_active(node u) const { return active[u]; }

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

    void k_core(int);

    // === Logs and checks ==================================

    void log() const;
    void log_stats(const string&) const;
    void apply_changes_to_sol(vector<set<graph::node>>&) const;

    bool is_connected() const;
    bool is_connected_complement() const;

    // === Weighted functions ================================
    void set_weight(const node&, const double&);
    double get_weight(const node&) const;
    double get_weight(const node_set&) const;

  private:
    node n;
    unsigned long m;
    vector<mod> delta;
    vector<node> deg;
    bitset<max_nodes> active;
    // BUG For some instances, this can be not enough
    vector<vector<node>> adj;
    vector<bitset<max_nodes>> adj_bool;
    vector<double> weights;

    unsigned long int add_edge(node, node);
    unsigned long int remove_edge(node, node);
    void do_conflict(node, node);
    void undo_conflict(node, node);
    void do_contract(node, node);
    void undo_contract(node, node);
    node check_deg(node) const;
    bool check_all_deg() const;
    void complement();
};

using node_set = graph::node_set;
using node = graph::node;
using edge = graph::edge;
#endif
