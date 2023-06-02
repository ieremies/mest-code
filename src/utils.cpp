#include "../incl/utils.hpp"

string to_string(const node_set& set)
{
    string s = "{";
    for (node u : set) {
        s += to_string(u) + ", ";
    }
    s += "}";
    return s;
}
