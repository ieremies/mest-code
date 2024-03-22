#ifndef FORMULATION_H_
#define FORMULATION_H_

/*
** The Formulation class is representation of the current state of the problem.
** It contains the underlying graph, the generated inpdendent sets and the
** cuts.
**
** We keep all generated independent sets and cuts in the formulation, even if
** there are innactive nodes on then. It will be the solver responsability to
** ignore those that are no longer usefull/true.
*/

#include "utils.hpp"

#define for_indep_set(M, s) \
    for (auto [s, i] = (M).first_act_indep_set(); (i) != -1; \
         tie(s, i) = (M).next_act_indep_set(i))
#define for_indep_set_with(M, n, s) \
    for (auto [s, i] = (M).first_act_indep_set_with(n); (i) != -1; \
         tie(s, i) = (M).next_act_indep_set_with(n, i))

#define for_cut(M, c) \
    for (auto [c, i] = (M).first_act_cut(); (i) < (M).get_cuts_size(); \
         tie(c, i) = (M).next_act_cut(i))

class formulation
{
    struct indep_set
    {
        node_set nodes;
        bool active;
    };
    struct cut
    {
        node a, b, c;
        bool active;
    };
    using indep_set_ptr = shared_ptr<indep_set>;
    using cut_ptr = shared_ptr<cut>;

    graph g;  // no one should change the graph directly besides formulation
    vector<indep_set_ptr> sets;
    vector<shared_ptr<cut>> cuts;

    // node_to_indep_set[i] is a list of independent sets that contain node i
    matrix<indep_set_ptr> node_to_indep_set;

    // node_to_cut[i] is a list of cuts that contain node i
    matrix<cut_ptr> node_to_cut;

    // similarity[i][j] is a list of independent sets that both i and j are in.
    matrix<vector<indep_set_ptr>> similarity;

  public:
    // === Constructors ========================================================
    formulation(const graph&, const color_sol&);

    // === Change =============================================================
    void add_indep_set(const node_set& s);
    void add_cut(const cut& c);

    // === Changing the graph =================================================
    // After each of these functions, the formulation should be updated
    void change(mod_type, node, node);
    void undo(mod_type, node, node);
    inline void set_weight(node n, cost c) { g.set_weight(n, c); }

    // === Getters ============================================================
    inline const graph& get_graph() const { return g; }
    inline unsigned long get_indep_sets_size() const { return sets.size(); }
    inline unsigned long get_cuts_size() const { return cuts.size(); }

    matrix<double> get_similarity(const color_sol&) const;

    // === Used for iteration =================================================
    pair<node_set, int> first_act_indep_set() const;
    pair<node_set, int> next_act_indep_set(int) const;

    pair<node_set, int> first_act_indep_set_with(node) const;
    pair<node_set, int> next_act_indep_set_with(node, int) const;

    pair<cut, int> first_act_cut() const;
    pair<cut, int> next_act_cut(int) const;

    // === Check functions ====================================================
    bool check_already_in(const node_set&) const;
    bool check_activation(const indep_set_ptr&) const;
    bool check_all() const;
};

#endif  // FORMULATION_H_
