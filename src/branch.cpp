
#include <cstdio>

#include "../incl/branch.hpp"

#include "../incl/utils.hpp"

pair<node, node> find_vertexes(const Graph& g,
                               const vector<node_set>& indep_set,
                               map<node_set, double> x_s)
{
    const double half = 0.5;

    vector<vector<double>> diff(g.get_n(), vector<double>(g.get_n(), 0));
    for (const node_set& set : indep_set) {
        for (node u : set) {
            CHECK_F(g.is_active(u),
                    "node that is not active is in indep sets.");
            for (node v : set) {
                CHECK_F(g.is_active(v),
                        "node that is not active is in indep sets.");
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

void Branch::branch(const Graph& g,
                    const vector<node_set>& indep_sets,
                    const map<node_set, double>& x_s,
                    const double& obj_val)
{
    LOG_SCOPE_F(INFO, "Branch::Branch");
    auto [u, v] = find_vertexes(g, indep_sets, x_s);
    if (u >= g.get_n() || v >= g.get_n()) {
        return;
    }

    LOG_F(INFO, "Adding branch on %d and %d", u, v);
    tree.push(Branch::node {false, false, obj_val, u, v, indep_sets});
}

vector<node_set> clean_sets(const mod_type& t,
                            const vector<node_set>& indep_sets,
                            const node& u,
                            const node& v)
{
    vector<node_set> ret;
    for (const node_set& set : indep_sets) {
        auto find_u = set.find(u) == set.end();
        auto find_v = set.find(v) == set.end();

        if (t == mod_type::conflict and (find_u or find_v)) {
            ret.push_back(set);
        }
        if (t == mod_type::contract and (find_u and find_v)) {
            ret.push_back(set);
        }
    }
    // FIXME there is a problem when a node is in no set.
    if (t == mod_type::contract) {
        ret.push_back({u});
    }
    return ret;
}

bool check_indep(const Graph& g, const vector<node_set>& indep_sets)
{
    for (const node_set& set : indep_sets) {
        for (node u : set) {
            for (node v : set) {
                if (g.get_incidency(u, v) != 0) {
                    LOG_F(ERROR, "%d %d are not independent", u, v);
                    return false;
                }
            }
        }
    }
    return true;
}

vector<node_set> Branch::next(Graph& g, const cost& upper_bound)
{
    LOG_SCOPE_F(INFO, "Branch::Next");

    if (tree.empty()) {
        return {};
    }

    // while worst than UB or completed
    Branch::node n = tree.top();

    // TODO tá certo esse +1?
    while (n.obj_val >= upper_bound + EPS
           or (n.conflict_done and n.contract_done))
    {
        tree.pop();

        if (n.contract_done) {
            g.undo(mod_type::contract, n.u, n.v);
        } else if (n.conflict_done) {
            g.undo(mod_type::conflict, n.u, n.v);
        }

        if (tree.empty()) {
            return {};
        }

        n = tree.top();
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

    mod_type t = mod_type::conflict;
    if (n.conflict_done) {
        t = mod_type::contract;
        n.contract_done = true;
    } else {
        t = mod_type::conflict;
        n.conflict_done = true;
    }

    LOG_F(INFO, "%s on %d <- %d", to_string(t), n.u, n.v);
    g.change(t, n.u, n.v);
    tree.push(n);

    vector<node_set> ret = clean_sets(t, n.indep_sets, n.u, n.v);
    DCHECK_F(check_indep(g, ret), "not independent set");

    return ret;
}
