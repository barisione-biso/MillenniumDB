#include "transitive_closure.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

TransitiveClosure::TransitiveClosure(std::size_t binding_size,
                                     BPlusTree<4>& bpt,
                                     Id start,
                                     Id end,
                                     ObjectId type,
                                     uint_fast32_t start_pos,
                                     uint_fast32_t end_pos,
                                     uint_fast32_t type_pos) :
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
    // min_ids[start_pos] and max_ids[start_pos] will be setted at next()

    // set start_object_id and add it to `open` and `visited`
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        visited.insert(start_object_id);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        visited.insert(start_object_id);
        open.push(start_object_id);
    }

    // set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*my_input)[end_var_id];
    }

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
            ObjectId child( child_record->ids[end_pos] );
            if (child == end_object_id) {
                queue<ObjectId> empty;
                open.swap(empty);
                return true;
            } else {
                if (visited.find(child) != visited.end()) {
                    visited.insert(child);
                    open.push(child);
                }
                child_record = it->next();
            }
        }
    }
    return false;
}


void TransitiveClosure::reset() {
    // empty open and visited
    queue<ObjectId> empty;
    open.swap(empty);

    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        visited.insert(start_object_id);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        visited.insert(start_object_id);
        open.push(start_object_id);
    }

    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*my_input)[end_var_id];
    }
}


void TransitiveClosure::analyze(int) const {
}
