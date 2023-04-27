#include "../incl/utils.hpp"
#include <stack>
#include <vector>

class BranchNode {
  public:
    Graph g;
    std::vector<NodeSet> indep_sets;

    BranchNode(const Graph &, const std::vector<NodeSet> &);
    ~BranchNode();
};

class Branch {
  private:
    std::stack<BranchNode *> tree;

  public:
    /*
    ** Create the branching tree with the initial graph and independent
    ** set.
    */
    Branch(const Graph &, const std::vector<NodeSet> &);

    /*
    ** Deletes the tree and all the remaining BranchNodes.
    */
    ~Branch();

    /*
    ** Receives the graph, the independent sets and each of those x_s values.
    ** Each x_s means if the independent set is in the solution or not.
    **
    ** Returns the number of branchs created (none if the solution is integer).
    */
    int branch(const Graph &, const std::vector<NodeSet> &,
               const std::vector<double> &x_s);

    /*
    ** Populates the graph and the independent sets with the next branch.
    */
    void next(Graph &, std::vector<NodeSet> &);
};
