#include "distinct_id_hash.h"

#include <iostream>

using namespace std;

DistinctIdHash::DistinctIdHash(unique_ptr<BindingIdIter> _child_iter, std::vector<VarId> projected_vars) :
    child_iter       (move(_child_iter)),
    projected_vars   (projected_vars),
    extendable_table (DistinctBindingHash<ObjectId>(projected_vars.size()))
    { }


void DistinctIdHash::begin(BindingId& parent_binding) {
    this->parent_binding = &parent_binding;
    child_iter->begin(parent_binding);
    current_tuple = std::vector<ObjectId>(projected_vars.size());
}


void DistinctIdHash::reset() {
    // TODO: now this method is never called, maybe in the future we may need to clear hash table
    child_iter->reset();
}


bool DistinctIdHash::next() {
    while (child_iter->next()) {
        // load current objects
        for (size_t i = 0; i < projected_vars.size(); i++) {
            current_tuple[i] = (*parent_binding)[projected_vars[i]];
        }
        if (current_tuple_distinct()) {
            return true;
        }
    }
    return false;
}


void DistinctIdHash::assign_nulls() {
    child_iter->assign_nulls();
}


bool DistinctIdHash::current_tuple_distinct() {
    bool is_new_tuple = !extendable_table.is_in_or_insert(current_tuple);
    return is_new_tuple;
}


void DistinctIdHash::analyze(int indent) const {
    std::cout << std::string(indent, ' ');
    std::cout << "DistinctIdHash()\n";
    child_iter->analyze(indent+2);
}
