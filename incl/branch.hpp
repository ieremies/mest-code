#include "../incl/utils.hpp"
#include <queue>
#include <vector>

// Struct to represent a node in the branch tree
typedef struct BranchNode {
    // The underlying graph
    Graph g;

    // The smallest known coloring
    int xi;                          // == len(indep_sets)
    std::vector<NodeSet> indep_sets; // only those being used

} BranchNode;

class Branch {
  private:
    // algguma forma de guardar a árvore de branching
    std::queue<BranchNode> tree;

  public:
    Branch();
    ~Branch();

    /*
    ** Run the branching strategy.
    ** Receives the model and uses the branching strategy to branch.
    ** Returns the number of nodes branched.
    */
    int branch(Graph &, Graph::NodeMap<double> &);

    /*
     ** Based on the state of the tree, returns the next node to be optimized.
     ** Receives the upper and lower bound known so far, in order to prune any
     ** unfruitful branch.
     */
    void next(double upper_bound, double lower_bound);
};
