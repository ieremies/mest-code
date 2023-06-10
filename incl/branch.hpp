#include <map>
#include <stack>
#include <vector>

#include "../incl/utils.hpp"

class Branch
{
  public:
    int branch(const Graph&,
               const vector<node_set>&,
               const map<node_set, double>& x_s,
               const double&);

    vector<node_set> next(Graph&, const cost&, cost&);

    struct node
    {
        bool conflict_done;
        bool contract_done;
        double obj_val;
        Graph::node u, v;
        vector<node_set> indep_sets;
    };
    Branch()
        : tree()
    {
    }

  private:
    stack<Branch::node> tree;
};
