#include "property_path_iddfs_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathIDDFSEnum::PropertyPathIDDFSEnum(BPlusTree<4>& type_from_to_edge,
                                     BPlusTree<4>& to_type_from_edge,
                                     Id start,
                                     VarId end,
                                     PathAutomaton automaton
                                     ) :
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathIDDFSEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    automaton.print();
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            auto start_pair = SearchState(automaton.start, start_object_id);
            open.push(start_pair);
            visited.insert(start_pair);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            auto start_pair = SearchState(automaton.start, start_object_id);
            open.push(start_pair);
            visited.insert(start_pair);
        }

        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        // pos 0 and 1 will be set at next()
    }
}


bool PropertyPathIDDFSEnum::next() {
    while (open.size() > 0) {
        auto current_state = open.top();
        open.pop();
        std::unique_ptr<BptIter<4>> it;
        for (const auto& transition : automaton.transitions[current_state.state]) {
            if (transition.inverse) {
                min_ids[0] = current_state.object_id.id;
                max_ids[0] = current_state.object_id.id;
                min_ids[1] = transition.label.id;
                max_ids[1] = transition.label.id;
                it = to_type_from_edge.get_range(
                    Record<4>(min_ids),
                    Record<4>(max_ids)
                );
            } else {
                min_ids[0] = transition.label.id;
                max_ids[0] = transition.label.id;
                min_ids[1] = current_state.object_id.id;
                max_ids[1] = current_state.object_id.id;
                it = type_from_to_edge.get_range(
                    Record<4>(min_ids),
                    Record<4>(max_ids)
                );
            }
            bpt_searches++;
            auto child_record = it->next();
            while (child_record != nullptr) {
                auto next_state = SearchState(transition.to, ObjectId(child_record->ids[2]));
                if (visited.find(next_state) == visited.end()) {
                    open.push(next_state);
                    visited.insert(next_state);
                }
                child_record = it->next();
            }
        }
        if (automaton.end.find(current_state.state) != automaton.end.end()) {
            results_found++;
            parent_binding->add(end, current_state.object_id);
            return true;
        }
    }
    return false;
}


void PropertyPathIDDFSEnum::reset() {
    // Empty open and visited
    stack<SearchState> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_pair = SearchState(automaton.start, start_object_id);
        open.push(start_pair);
        visited.insert(start_pair);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        auto start_pair = SearchState(automaton.start, start_object_id);
        open.push(start_pair);
        visited.insert(start_pair);
    }
}


void PropertyPathIDDFSEnum::assign_nulls() { }


void PropertyPathIDDFSEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathIDDFSEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
