#include "transitive_closure_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

TransitiveClosureEnum::TransitiveClosureEnum(std::size_t /*binding_size*/,
                                             BPlusTree<4>& bpt,
                                             Id start,
                                             VarId end,
                                             ObjectId type,
                                             uint_fast32_t start_pos,
                                             uint_fast32_t type_pos) :
    bpt           (bpt),
    start         (start),
    end           (end),
    type          (type),
    start_pos     (start_pos),
    type_pos      (type_pos) { }


void TransitiveClosureEnum::begin(BindingId& parent_binding, bool /* parent_has_next */) {
    this->parent_binding = &parent_binding;
    min_ids[type_pos] = type.id;
    max_ids[type_pos] = type.id;
    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;

    // Set start_object_id and add it to `open`
    if (std::holds_alternative<ObjectId>(start)) {
        start_object_id = std::get<ObjectId>(start);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        start_object_id = (parent_binding)[start_var_id];
        open.push(start_object_id);
    }

    child_record = nullptr;
    self_reference = false;
}


bool TransitiveClosureEnum::next() {
    while ((open.size() > 0) || self_reference) {
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
            ++bpt_searches;
        }
        // Find next node
        self_reference = false;
        child_record = it->next();
        while (child_record != nullptr){
            ObjectId child( child_record->ids[2] );
            if (visited.find(child) == visited.end()) {
                visited.insert(child);
                parent_binding->add(end, child);
                if (child != start_object_id) {
                    open.push(child);
                } else {
                    self_reference = true;
                }
                ++results_found;
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
        start_object_id = std::get<ObjectId>(start);
        open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        start_object_id = (*parent_binding)[start_var_id];
        open.push(start_object_id);
    }
}


void TransitiveClosureEnum::assign_nulls() {
    parent_binding->add(end, ObjectId::get_null());
}


void TransitiveClosureEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "TransitiveClosurEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
