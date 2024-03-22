#include "formulation.hpp"

#include "utils.hpp"

// === Constructors ========================================================
formulation::formulation(const graph& g, const color_sol& s)
    : g(g)
    , node_to_indep_set(g.get_n())
    , node_to_cut(g.get_n())
    , similarity(g.get_n(), vector<vector<indep_set_ptr>>(g.get_n()))
{
    for (const auto& [set, x] : s.x_sets) {
        add_indep_set(set);
    }
    for (node n = 0; n < g.get_n(); n++) {
        if (!check_already_in({n})) {
            add_indep_set({n});
        }
    }

    DCHECK_F(check_all(), "Formulation is not valid.");
}

// === Change =============================================================
void formulation::add_indep_set(const node_set& s)
{
    // At the time of adition, the set has to be valid
    DCHECK_F(check_indep_set(g, s), "Set %s is not valid.", str(s));
    // check if its not already in the formulation
    DCHECK_F(check_already_in(s) == false,
             "Set %s is already in the formulation.",
             str(s));

    indep_set_ptr const sp_s = make_shared<indep_set>(indep_set {s, true});

    sets.push_back(sp_s);
    for (const node& n : s) {
        node_to_indep_set[n].push_back(sp_s);
        sp_s->active &= g.is_active(n);
    }
    for (const node& u : s) {
        for (const node& v : s) {
            if (u == v) {
                continue;
            }
            similarity[u][v].push_back(sp_s);
        }
    }

    DCHECK_F(check_all(), "Formulation is not valid.");
}

// === Changing the graph =================================================
void formulation::change(mod_type t, node a, node b)
{
    g.change(t, a, b);
    if (t == mod_type::conflict) {
        for (const indep_set_ptr& sp_s : similarity[a][b]) {
            sp_s->active = false;
            // TODO instead create two new indep_sets
        }
    } else {
        for (const indep_set_ptr& sp_s : node_to_indep_set[b]) {
            sp_s->active = false;
            // TODO instead create a new indep_set without b
        }
        for (const indep_set_ptr& sp_s : node_to_indep_set[a]) {
            sp_s->active = check_activation(sp_s);
        }
    }
    // TODO handle cuts
    DCHECK_F(check_all(), "Formulation is not valid.");
}

void formulation::undo(mod_type t, node a, node b)
{
    g.undo(t, a, b);
    if (t == mod_type::conflict) {
        for (const indep_set_ptr& sp_s : similarity[a][b]) {
            sp_s->active = check_activation(sp_s);
        }
    } else {
        for (const indep_set_ptr& sp_s : node_to_indep_set[b]) {
            sp_s->active = check_activation(sp_s);
        }
        for (const indep_set_ptr& sp_s : node_to_indep_set[a]) {
            sp_s->active = check_activation(sp_s);
        }
    }
    // TODO handle cuts
    DCHECK_F(check_all(), "Formulation is not valid.");
}

// === Getters ============================================================
matrix<double> formulation::get_similarity(const color_sol& s) const
{
    matrix<double> sim_value(g.get_n(), vector<double>(g.get_n(), 0.0));
    for_pair_nodes(g, u, v) {
        for (const indep_set_ptr& sp_s : similarity[u][v]) {
            if (not sp_s->active) {
                continue;
            }

            DCHECK_F(g.get_adjacency(u, v) == 0,
                     "(%d, %d) adj, but %s active!",
                     u,
                     v,
                     str(sp_s->nodes));
            sim_value[u][v] += s.x_sets.at(sp_s->nodes);
            sim_value[v][u] += s.x_sets.at(sp_s->nodes);
        }
    }
    // Log similarity matrix
    return sim_value;
}

// === Used for iteration =================================================
pair<node_set, int> formulation::first_act_indep_set() const
{
    return next_act_indep_set(-1);
}

pair<node_set, int> formulation::next_act_indep_set(int i) const
{
    for (i++; i < static_cast<int>(sets.size()); i++) {
        if (sets[i]->active) {
            return make_pair(sets[i]->nodes, i);
        }
    }
    return make_pair(node_set(), -1);
}

pair<node_set, int> formulation::first_act_indep_set_with(node n) const
{
    return next_act_indep_set_with(n, -1);
}

pair<node_set, int> formulation::next_act_indep_set_with(node n, int i) const
{
    for (i++; i < static_cast<int>(node_to_indep_set[n].size()); i++) {
        if (node_to_indep_set[n][i]->active) {
            return make_pair(node_to_indep_set[n][i]->nodes, i);
        }
    }
    return make_pair(node_set(), -1);
}

// === Check functions ========================================================
bool formulation::check_activation(const indep_set_ptr& sp_s) const
{
    for (const node& n : sp_s->nodes) {
        if (not g.is_active(n)) {
            return false;
        }
        for_adj(g, n, m) {
            if (sp_s->nodes.count(m) > 0) {
                return false;
            }
        }
    }
    return true;
}

bool formulation::check_already_in(const node_set& s) const
{
    for (const indep_set_ptr& sp_s : sets) {
        if (sp_s->nodes == s) {
            return true;
        }
    }
    return false;
}

bool formulation::check_all() const
{
    for (const indep_set_ptr& sp_s : sets) {
        // check if sp_s->activation == check_activation(sp_s)
        if (sp_s->active != check_activation(sp_s)) {
            LOG_F(WARNING, "Set %s has wrong activation.", str(sp_s->nodes));
            return false;
        }
        // check if sp_s is in node_to_indep_set of each vertex
        for (const node& n : sp_s->nodes) {
            if (find(node_to_indep_set[n].begin(),
                     node_to_indep_set[n].end(),
                     sp_s)
                == node_to_indep_set[n].end())
            {
                LOG_F(WARNING,
                      "Set %s is not in node_to_indep_set of %d.",
                      str(sp_s->nodes),
                      n);
                return false;
            }
        }
        // check if sp_s is in similarity of each pair of nodes
        for (const node& u : sp_s->nodes) {
            for (const node& v : sp_s->nodes) {
                if (u == v) {
                    continue;
                }
                if (find(similarity[u][v].begin(), similarity[u][v].end(), sp_s)
                    == similarity[u][v].end())
                {
                    LOG_F(WARNING,
                          "Set %s is not in similarity of %d and %d.",
                          str(sp_s->nodes),
                          u,
                          v);
                    return false;
                }
            }
        }
    }
    return true;
}
