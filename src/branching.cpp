
#include <cstdio>

#include "branching.hpp"

#include "utils.hpp"

#define LOG_B_NODE(node) \
    LOG_F(INFO, \
          "%s on %d <- %d (lb: %f)", \
          ::to_string((node).current_mod).c_str(), \
          (node).u, \
          (node).v, \
          (node).lower_bound);

mod_type branching::node::next()
{
    current_mod =
        current_mod == mod_type::none ? mod_type::conflict : mod_type::contract;
    return current_mod;
}

pair<node, node> find_vertexes(const formulation& form, const color_sol& sol)
{
    LOG_SCOPE_FUNCTION(INFO);
    matrix<double> const sim = form.get_similarity(sol);
    double min_diff = INFINITY;
    node const n = form.get_graph().get_n();
    pair<node, node> min_pair = {n, n};
    for_pair_nodes(form.get_graph(), u, v) {
        double const diff = abs(sim[u][v] - 0.5);
        if (diff < min_diff) {
            min_diff = diff;
            min_pair = {u, v};

            if (min_diff == 0.0) {
                return min_pair;
            }
        }
    }
    LOG_F(INFO,
          "Found (%d %d) diff %f",
          min_pair.first,
          min_pair.second,
          min_diff);
    return min_pair;
}

/*
** Function that determines wheter or not to branch.
** If so, add the branch to the "tree" (stack).
*/
void branching::branch(const formulation& form, const color_sol& sol)
{
    LOG_SCOPE_FUNCTION(INFO);
    auto [u, v] = find_vertexes(form, sol);
    if (u >= form.get_graph().get_n() || v >= form.get_graph().get_n()) {
        return;
    }

    branching::node const n {mod_type::none, sol.cost, u, v};
    LOG_B_NODE(n);
    tree.push(n);
}

bool branching::next(formulation& form, const cost& upper_bound)
{
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Stack size: %lu | Visited: %ld", tree.size(), visited);

    if (tree.empty()) {
        return false;
    }
    branching::node n = tree.top();

    while (n.lower_bound > upper_bound + EPS  // if it's worse then UB
           or n.current_mod == mod_type::contract)  // or has been completed
    {
        tree.pop();

        // undo the last done
        if (n.current_mod == mod_type::contract) {
            form.undo(mod_type::contract, n.u, n.v);
        } else if (n.current_mod == mod_type::conflict) {
            form.undo(mod_type::conflict, n.u, n.v);
        }

        if (tree.empty()) {
            LOG_F(INFO, "Stack is empty");
            return false;
        }
        n = tree.top();
    }

    tree.pop();
    // If any mod has been done, it's time to undo it.
    if (n.current_mod == mod_type::conflict) {
        form.undo(mod_type::conflict, n.u, n.v);
    }

    n.next() == mod_type::contract ? form.change(mod_type::contract, n.u, n.v)
                                   : form.change(mod_type::conflict, n.u, n.v);
    tree.push(n);
    visited++;

    LOG_B_NODE(n);

    return true;
}
