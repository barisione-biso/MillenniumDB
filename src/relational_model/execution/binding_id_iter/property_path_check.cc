#include "property_path_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

// from_to_type_edge
// to_type_from_edge
// type_from_to_edge

// (start)=[]=>(end)
// (?x)=[]=>(N) === (N)=[^]=>(?x)
// - Forward: type_from_to_edge
// - Inverse: to_type_from_edge

PropertyPathCheck::PropertyPathCheck(BPlusTree<4>& type_from_to_edge,
                                     BPlusTree<4>& to_type_from_edge,
                                     Id start,
                                     Id end,
                                     PathAutomaton automaton) :
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathCheck::begin(BindingId& parent_binding, bool /* parent_has_next */) {
    this->parent_binding = &parent_binding;
    // Add inital state to queue
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_pair = Pair(automaton.start, start_object_id);
        open.push(start_pair);
        visited.insert(start_pair);
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = parent_binding[start_var_id];
        auto start_pair = Pair(automaton.start, start_object_id);
        open.push(start_pair);
        visited.insert(start_pair);
    }

    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = parent_binding[end_var_id];
    }

    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;
    // pos 0 and 1 will be set at next()
}


bool PropertyPathCheck::next() {
    while (open.size() > 0) {
        auto& current_pair = open.front();
        if (automaton.end.find(current_pair.state) != automaton.end.end() // check if state is final
            && current_pair.object_id == end_object_id)                   // check node
        {
            queue<Pair> empty;
            open.swap(empty);
            return true;
        } else if (visited.find(current_pair) == visited.end()) {
            for (const auto& transition : automaton.transitions[current_pair.state]) {
                if (transition.inverse) {
                    min_ids[0] = current_pair.object_id.id;
                    max_ids[0] = current_pair.object_id.id;
                    min_ids[1] = transition.label.id;
                    max_ids[1] = transition.label.id;
                    it = to_type_from_edge.get_range(
                        Record<4>(min_ids),
                        Record<4>(max_ids)
                    );
                } else {
                    min_ids[0] = transition.label.id;
                    max_ids[0] = transition.label.id;
                    min_ids[1] = current_pair.object_id.id;
                    max_ids[1] = current_pair.object_id.id;
                    it = type_from_to_edge.get_range(
                        Record<4>(min_ids),
                        Record<4>(max_ids)
                    );
                }
                auto child_record = it->next();
                while (child_record != nullptr) {
                    open.push( Pair(transition.to, ObjectId(child_record->ids[3])) );
                }

            }
            visited.insert(current_pair);
            open.pop();
        }
    }
    return false;
}


void PropertyPathCheck::reset() {
    // Empty open and visited
    queue<Pair> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.push(Pair(automaton.start, start_object_id));
    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.push(Pair(automaton.start, start_object_id));
    }

    // Set end_object_id
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
