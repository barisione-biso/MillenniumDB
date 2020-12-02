#include "transitive_closure.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;


TransitiveClosure::TransitiveClosure(std::size_t binding_size, BPlusTree<4>& bpt, ObjectId start, ObjectId end, ObjectId type,
                                     uint_fast32_t start_pos, uint_fast32_t end_pos, uint_fast32_t type_pos) :
    BindingIdIter (binding_size),
    bpt           (bpt),
    start         (start),
    end           (end),
    type          (type),
    start_pos     (start_pos),
    end_pos       (end_pos),
    type_pos      (type_pos) { }


BindingId& TransitiveClosure::begin(BindingId& input) {
    my_input = &input;
    min_ids[type_pos] = type.id;
    max_ids[type_pos] = type.id;
    min_ids[end_pos] = 0;
    max_ids[end_pos] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;

    // BFS Initialization
    visited.insert(start);
    open.push(start);

    return my_binding;
}


bool TransitiveClosure::next() {
    // BFS (base case)
    while (open.size() > 0) {
        auto current = open.front();
        open.pop();
        min_ids[start_pos] = current.id;
        max_ids[start_pos] = current.id;
        it = bpt.get_range(
            Record<4>(min_ids),
            Record<4>(max_ids)
        );
        auto child_record = it->next();
        while (child_record != nullptr){
            auto child = child_record->ids[end_pos];
            if (child == end.id) {
                queue<ObjectId> empty;
                open.swap(empty);
                return true;
            } else {
                if (visited.find(ObjectId(child)) != visited.end()) {
                    visited.insert(ObjectId(child));
                    open.push(ObjectId(child));
                }
                child_record = it->next();
            }
        }
    }
    return false;
}


void TransitiveClosure::reset() {
}


void TransitiveClosure::analyze(int) const {
}
