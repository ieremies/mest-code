#include <map>
#include <stack>
#include <vector>

#include "../incl/utils.hpp"

class Branch
{
  public:
    struct node
    {
        bool conflict_done;
        bool contract_done;
        cost obj_val;
        Graph::node u, v;
        vector<node_set> indep_sets;
    };

    Branch() = default;

    /*
    ** Function that determines wheter or not to branch.
    ** It receives an instance of the problem with some independent sets with
    ** also the current solution (x_s) and the current objective value
    ** (obj_val).
    */
    void branch(const Graph&,
                const vector<node_set>&,
                const map<node_set, cost>&,
                const cost&);

    /*
    ** Goes to the next node in the branch tree.
    ** It undoes and does the necessary operations to the graph and the
    ** independent sets to go to the next node.
    ** It returns the independent sets of the next node.
    */
    vector<node_set> next(Graph&, const cost&);

  private:
    stack<Branch::node> tree;
};
