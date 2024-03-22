#include <map>
#include <stack>
#include <vector>

#include "formulation.hpp"
#include "utils.hpp"

class branching
{
  public:
    struct node
    {
        mod_type current_mod;  // none -> conflict -> contract
        cost lower_bound;
        graph::node u, v;

        mod_type next();
    };

    branching() = default;

    /*
    ** Function that determines wheter or not to branch.
    ** The current solution is lower bound for all future branchs.
    */
    void branch(const formulation&, const color_sol&);

    /*
    ** Goes to the next node in the branch tree.
    ** It does all the necessary changes to the graph and calls the formulation
    ** to update its active sets and cuts.
    **
    ** The received cost is the upper bound for the next node. All nodes with
    ** lower bound greater then this cost are pruned.
    **
    ** Return true if there is a next node to be solved, false otherwise.
    */
    bool next(formulation&, const cost&);

  private:
    stack<branching::node> tree;
    unsigned long visited = 0;
};
