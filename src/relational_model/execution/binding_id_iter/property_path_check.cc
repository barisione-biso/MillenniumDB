#include "property_path_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathCheck::PropertyPathCheck(
                                    BPlusTree<4>& bpt,
                                    Id start,
                                    Id end,
                                    uint_fast32_t start_pos,
                                    uint_fast32_t type_pos,
                                    PathAutomaton automaton) :
    bpt           (bpt),
    start         (start),
    end           (end),
    automaton     (automaton),
    start_pos     (start_pos),
    type_pos      (type_pos) { }


void PropertyPathCheck::begin(BindingId& parent_binding, bool /* parent_has_next */) {
    // Add inital state to queue
    this->parent_binding = &parent_binding;
    // Set start_object_id and add it to `open` and `visited`
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        //visited.insert(start_object_id);
        open.push(Pair(automaton.start, start_object_id));
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (parent_binding)[start_var_id];
        //visited.insert(start_object_id);
        open.push(Pair(automaton.start, start_object_id));
    }

    //min_ids[type_pos] = type.id;
    //max_ids[type_pos] = type.id;
    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;
    // min_ids[start_pos] and max_ids[start_pos] will be set at next()



    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (parent_binding)[end_var_id];
    }
}


bool PropertyPathCheck::next() {
    while (open.size() > 0) {
        auto current = open.front();
        open.pop();
        //min_ids[start_pos] = current.id;
        //max_ids[start_pos] = current.id;
        it = bpt.get_range(
            Record<4>(min_ids),
            Record<4>(max_ids)
        );
        ++bpt_searches;

        auto child_record = it->next();
        while (child_record != nullptr){
            ObjectId child( child_record->ids[2] );
            //if (child == end_object_id) {
            //    queue<ObjectId> empty;
            //    open.swap(empty);
            //    ++results_found;
            //    return true;
            //} else {
            //    if (visited.find(child) == visited.end()) {
            //        visited.insert(child);
            //        open.push(child);
            //    }
            //    child_record = it->next();
            //}
        }
    }
    return false;
}


void PropertyPathCheck::reset() {
    // Empty open and visited
    queue<ObjectId> empty;
    //open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        //visited.insert(start_object_id);
        //open.push(start_object_id);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        //visited.insert(start_object_id);
        //open.push(start_object_id);
    }

    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*parent_binding)[end_var_id];
    }
}


void PropertyPathCheck::assign_nulls() { }


void PropertyPathCheck::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "TransitiveClosurCheck(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
