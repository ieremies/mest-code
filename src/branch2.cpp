
#include <cstdio>

#include "../incl/branch.hpp"

#include "../lib/loguru.hpp"

pair<node, node> find_vertexes(const Graph& g,
                               const vector<node_set>& indep_set,
                               map<node_set, double> x_s)
{
    const double half = 0.5;

    vector<vector<double>> diff(g.get_n(), vector<double>(g.get_n(), 0));
    for (const node_set& set : indep_set) {
        for (node u : set) {
            CHECK_F(g.is_active(u), "node is not active.");
            for (node v : set) {
                CHECK_F(g.is_active(v), "node is not active.");
                if (u < v) {
                    diff[u][v] += half - abs(half - x_s.at(set));
                }
            }
        }
    }

    double max = 0;
    node u = 0;
    node v = 0;
    for_nodes(g, _u)
        for_nodes(g, _v)
            if (max < diff[_u][_v] and g.get_incidency(_u, _v) == 0) {
                max = diff[_u][_v];
                u = _u;
                v = _v;
            }

    if (max <= 0 + EPS) {
        return make_pair(g.get_n(), g.get_n());
    }

    return make_pair(u, v);
}

int Branch::branch(const Graph& g,
                   const vector<node_set>& indep_sets,
                   const map<node_set, double>& x_s,
                   const double& obj_val)
{
    LOG_SCOPE_F(INFO, "Branch::Branch");
    auto [u, v] = find_vertexes(g, indep_sets, x_s);
    if (u >= g.get_n() || v >= g.get_n()) {
        return 0;
    }

    LOG_F(INFO, "Adding branch on %d and %d", u, v);
    tree.push(Branch::node {false, false, obj_val, u, v, indep_sets});

    return 0;
}

vector<node_set> clean_sets_conflict(const vector<node_set>& indep_sets,
                                     const node& u,
                                     const node& v)
{
    vector<node_set> ret;
    for (const node_set& set : indep_sets) {
        if (set.find(u) == set.end() or set.find(v) == set.end()) {
            ret.push_back(set);
        }
    }
    return ret;
}

vector<node_set> clean_sets_contract(const vector<node_set>& indep_sets,
                                     const node& u,
                                     const node& v)
{
    vector<node_set> ret;
    for (const node_set& set : indep_sets) {
        auto it_u = set.find(u);
        auto it_v = set.find(v);

        if (it_u == set.end() and it_v == set.end()) {
            ret.push_back(set);
        }
    }
    ret.push_back({u});
    return ret;
}
/*
** Um nó é o próximo se sua obj_value é menor que o upper_bound e
** algum dos dois branchs ainda precisam ser feitos.
**
** Se falhar por causa da obj_value, aidna precisa desfazer o último
** branch.
*/
vector<node_set> Branch::next(Graph& g, const double& upper_bound)
{
    LOG_SCOPE_F(INFO, "Branch::Next");
    if (tree.empty()) {
        return {};
    }

    // while worst than UB or completed
    Branch::node n = tree.top();
    for (;
         n.obj_val >= upper_bound + EPS || (n.conflict_done && n.contract_done);
         n = tree.top())
    {
        if (n.contract_done) {
            g.undo(mod_type::contract, n.u, n.v);
        } else if (n.conflict_done) {
            g.undo(mod_type::conflict, n.u, n.v);
        }
        tree.pop();
        if (tree.empty()) {
            return {};
        }
    }
    tree.pop();

    LOG_F(INFO,
          "Looking node {%d,%d}: conflict %d | contract %d (sol: %lf)",
          n.u,
          n.v,
          n.conflict_done,
          n.contract_done,
          n.obj_val);
    // If any mod has been done, it's time to undo it.
    if (n.contract_done) {
        g.undo(mod_type::contract, n.u, n.v);
    } else if (n.conflict_done) {
        g.undo(mod_type::conflict, n.u, n.v);
    }

    vector<node_set> ret;
    if (not n.conflict_done) {
        // Se ainda não foi feito o conflito, essa é a hora
        g.change(mod_type::conflict, n.u, n.v);
        n.conflict_done = true;
        LOG_F(INFO, "Conflict on %d -- %d", n.u, n.v);
        tree.push(n);
        ret = clean_sets_conflict(n.indep_sets, n.u, n.v);
    } else {
        // Caso já tenha sido feito, fazemos o contract
        g.change(mod_type::contract, n.u, n.v);
        n.contract_done = true;
        LOG_F(INFO, "Contract on %d <- %d", n.u, n.v);
        tree.push(n);
        ret = clean_sets_contract(n.indep_sets, n.u, n.v);
    }
    for (const node_set& set : ret) {
        for (Graph::node n1 : set) {
            for (Graph::node n2 : set) {
                CHECK_F(n1 == n2 or g.get_incidency(n1, n2) == 0,
                        "Not independent set.");
            }
        }
    }
    return ret;
}
