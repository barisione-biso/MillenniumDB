#include "transitive_closure_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

TransitiveClosureEnum::TransitiveClosureEnum(std::size_t binding_size,
                                             BPlusTree<4>& bpt,
                                             Id start,
                                             VarId end,
                                             ObjectId type,
                                             uint_fast32_t start_pos,
                                             uint_fast32_t type_pos) :
    BindingIdIter (binding_size),
    bpt           (bpt),
    start         (start),
    end           (end),
    type          (type),
    start_pos     (start_pos),
    type_pos      (type_pos) { }


BindingId& TransitiveClosureEnum::begin(BindingId& input) {
    my_input = &input;
    min_ids[type_pos] = type.id;
    max_ids[type_pos] = type.id;
    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;

    // set start_object_id and add it to `open` and `visited`
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        // visited.insert(start_object_id); // TODO: First node can point to itself (check)
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        // visited.insert(start_object_id); // TODO: First node can point to itself (check)
        open.push(start_object_id);
    }
    child_record = nullptr;
    return my_binding;
}


bool TransitiveClosureEnum::next() {
    while (open.size() > 0) {
        // Change iterator to next root node
        if (child_record == nullptr) {
            auto current = open.front();
            open.pop();
            min_ids[start_pos] = current.id;
            max_ids[start_pos] = current.id;
            it = bpt.get_range(
                Record<4>(min_ids),
                Record<4>(max_ids)
            );
        }
        // Find next node
        child_record = it->next();
        while (child_record != nullptr){
            ObjectId child( child_record->ids[2] );
            if (visited.find(child) == visited.end()) {
                visited.insert(child);
                open.push(child);
                my_binding.add_all(*my_input);
                my_binding.add(end, child);
                return true;
            }
            child_record = it->next();
        }
    }
    return false;
}


void TransitiveClosureEnum::reset() {
    // empty open and visited
    queue<ObjectId> empty;
    open.swap(empty);
    child_record = nullptr;

    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        // visited.insert(start_object_id);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        // visited.insert(start_object_id);
        open.push(start_object_id);
    }
}


void TransitiveClosureEnum::analyze(int) const {
}
