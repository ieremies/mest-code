#include "branch_cut_price.hpp"

#include "branching.hpp"
#include "formulation.hpp"
#include "heuristic.hpp"
#include "solver.hpp"

color_sol branch_cut_price::solve(const graph& orig_g, const color_sol& orig_s)
{
    formulation form(orig_g, orig_s);
    color_sol upper_bound = orig_s;
    branching tree;

    while (true) {
#ifndef NDEBUG
        check_connectivity(form.get_graph());
        check_universal(form.get_graph());
#endif

        solver solver(form);
        color_sol const sol = solver.solve();
        form = solver.get_formulation();
        LOG_F(INFO, "Solved with value %f", sol.cost);

        if (sol.is_integral() and sol.cost + EPS < upper_bound.cost) {
            upper_bound = sol;
            LOG_F(INFO, "%s", sol.str(form.get_graph()));
        }

        if (form.get_graph().get_n_mods() % 10 == 0) {
            const color_sol new_upper_bound = heuristic(form.get_graph());
            if (new_upper_bound.cost + EPS < upper_bound.cost) {
                upper_bound = new_upper_bound;
                LOG_F(INFO, "New upper bound %f", upper_bound.cost);
            }
        }
        // TODO heuristica de arredondamento

        if (ceil(sol.cost) < upper_bound.cost) {
            tree.branch(form, sol);
        }

        if (not tree.next(form, upper_bound.cost)) {
            break;
        }
    }

    return upper_bound;
}
