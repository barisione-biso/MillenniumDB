#include "property_path_bfs_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;


PropertyPathBFSCheck::PropertyPathBFSCheck(BPlusTree<4>& type_from_to_edge,
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


void PropertyPathBFSCheck::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            auto start_state = SearchState(automaton.start, start_object_id);
            open.push(start_state);
            visited.insert(start_state);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            auto start_state = SearchState(automaton.start, start_object_id);
            open.push(start_state);
            visited.insert(start_state);
        }

        // Set end_object_id
        if (std::holds_alternative<ObjectId>(end)) {
            end_object_id = std::get<ObjectId>(end);
        } else {
            auto end_var_id = std::get<VarId>(end);
            end_object_id = parent_binding[end_var_id];
        }
        is_first = true;
        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        // pos 0 and 1 will be set at next()
    }
}


bool PropertyPathBFSCheck::next() {
    // Check if first node is end state
    if (is_first) {
        is_first = false;
        if (automaton.start_is_final && open.front().object_id == end_object_id) {
            queue<SearchState> empty;
            open.swap(empty);
            results_found++;
            return true;
        }
    }
    // BFS classic implementation
    while (open.size() > 0) {
        auto& current_state = open.front();
        // Expand state. Only visit nodes that automatons transitions indicates
        while (current_state.transition < automaton.transitions[current_state.state].size()) {
            const auto& transition = automaton.transitions[current_state.state][current_state.transition];
            set_iter(transition, current_state);

            // Explore matches nodes
            auto child_record = iter->next();
            while (child_record != nullptr) {
                auto next_state = SearchState(transition.to, ObjectId(child_record->ids[2]));

                // Check if next_state is final
                if (next_state.state == automaton.final_state
                    && next_state.object_id == end_object_id )
                {
                    queue<SearchState> empty;
                    open.swap(empty);
                    results_found++;
                    return true;
                }
                // Add to visited and queue
                if (visited.find(next_state) == visited.end()) {
                    open.push(next_state);
                    visited.insert(next_state);
                }
                child_record = iter->next();
            }
        // Search to next transition
        current_state.transition++;
        }
        open.pop();
    }
    return false;
}


void PropertyPathBFSCheck::set_iter(
    const TransitionId& transition,
    const SearchState& current_state) {
    // Get iter from correct bpt_tree according to inverse attribute
    if (transition.inverse) {
        min_ids[0] = current_state.object_id.id;
        max_ids[0] = current_state.object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        iter = to_type_from_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = current_state.object_id.id;
        max_ids[1] = current_state.object_id.id;
        iter = type_from_to_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
    bpt_searches++;
}


void PropertyPathBFSCheck::reset() {
    // Empty open and visited
    queue<SearchState> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_state = SearchState(automaton.start, start_object_id);
        open.push(start_state);
        visited.insert(start_state);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        auto start_state = SearchState(automaton.start, start_object_id);
        open.push(start_state);
        visited.insert(start_state);
    }

    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*parent_binding)[end_var_id];
    }
}


void PropertyPathBFSCheck::assign_nulls() { }


void PropertyPathBFSCheck::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSCheck(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
