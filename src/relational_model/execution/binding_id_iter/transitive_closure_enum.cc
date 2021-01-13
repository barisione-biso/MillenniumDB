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

    // Set start_object_id and add it to `open`
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        open.push(start_object_id);
    }

    child_record = nullptr;
    self_reference = false;
    return my_binding;
}


bool TransitiveClosureEnum::next() {
    while ((open.size() > 0) || self_reference) {
        outer_while_count ++;
        // Change iterator to next root node
        if (child_record == nullptr) {
            auto current = open.front();
            open.pop();
            // cout << "Open Size: " << open.size() << "\n";
            min_ids[start_pos] = current.id;
            max_ids[start_pos] = current.id;
            it = bpt.get_range(
                Record<4>(min_ids),
                Record<4>(max_ids)
            );
            bpt_searches ++;
        }
        // Find next node
        self_reference = false;
        child_record = it->next();
        while (child_record != nullptr){
            inner_while_count ++;
            ObjectId child( child_record->ids[2] );
            if (visited.find(child) == visited.end()) {
                visited.insert(child);
                // cout << "Visited Size: " << visited.size() << "\n";
                my_binding.add_all(*my_input);
                my_binding.add(end, child);
                if (child != std::get<ObjectId>(start)) {
                    open.push(child);
                    // cout << "Open Size: " << open.size() << "\n";
                } else {
                    self_reference = true;
                }
                return true;
            }
            child_record = it->next();
        }
    }
    return false;
}


void TransitiveClosureEnum::reset() {
    // Empty open, visited, current node
    queue<ObjectId> empty;
    open.swap(empty);
    visited.clear();
    child_record = nullptr;
    self_reference = false;

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*my_input)[start_var_id];
        open.push(start_object_id);
    }
}


void TransitiveClosureEnum::analyze(int) const {
    cout << "Outer While Count: " << outer_while_count << "\n";
    cout << "Inner While Count: " << inner_while_count << "\n";
    cout << "BPT Searches: " << bpt_searches << "\n";
}