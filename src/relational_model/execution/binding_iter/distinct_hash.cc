#include "distinct_hash.h"

#include <iostream>

using namespace std;

DistinctHash::DistinctHash(unique_ptr<BindingIter> _child_iter, std::vector<VarId> projected_vars) :
    child_iter       (move(_child_iter)),
    child_binding    (child_iter->get_binding()),
    projected_vars   (projected_vars),
    extendable_table (DistinctBindingHash<GraphObject>(projected_vars.size()))
    { }


void DistinctHash::begin() {
    child_iter->begin();
    current_tuple = std::vector<GraphObject>(projected_vars.size());
}


bool DistinctHash::next() {
    while (child_iter->next()) {
        // load current objects
        for (size_t i = 0; i < projected_vars.size(); i++) {
            current_tuple[i] = child_binding[projected_vars[i]];
        }
        if (current_tuple_distinct()) {
            return true;
        }
    }
    return false;
}


bool DistinctHash::current_tuple_distinct() {
    bool is_new_tuple = !extendable_table.is_in_or_insert(current_tuple);
    return is_new_tuple;
}


void DistinctHash::analyze(int indent) const {
    std::cout << std::string(indent, ' ');
    std::cout << "DistinctHash()\n";
    child_iter->analyze(indent+2);
}
