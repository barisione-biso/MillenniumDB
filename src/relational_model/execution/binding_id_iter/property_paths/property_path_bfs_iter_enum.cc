#include "property_path_bfs_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;


bool State::has_next(std::unordered_set<StateKey, StateKeyHasher>& visited) {
    if (iter == nullptr) { // if is first time that State is explore
        current_transtion = 0;
        // Check automaton has transitions
        if (current_transtion >= automaton.transitions[automaton_start_state].size()) {
            return false;
        }
        // Constructs iter
        set_iter();
    }
    // Iterate over automaton_start state transtions
    while (current_transtion < automaton.transitions[automaton_start_state].size()) {
        auto& transition = automaton.transitions[automaton_start_state][current_transtion];
        auto child_record = iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto next_object_id = ObjectId(child_record->ids[2]);
            auto next_automaton_state = transition.to;
            auto next_state_key = StateKey(next_automaton_state, next_object_id);
            // Check child is not already visited
            if (visited.find(next_state_key) == visited.end()) {
                visited.insert(next_state_key);
                // Update nexrt state settings
                reached_automaton_state = transition.to;
                reached_object_id = next_object_id;
                return true;
            }
            child_record = iter->next();
        }
        // Constructs new iter
        current_transtion++;
        set_iter();
    }
    return false;
}


void State::set_iter() {
    // Gets current transition object from automaton
    auto& transition = automaton.transitions[automaton_start_state][current_transtion];

    // Gets iter from correct bpt with transition.inverse
    if (transition.inverse) {
        min_ids[0] = start_object_id.id;
        max_ids[0] = start_object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        iter = bpt_inverse.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = start_object_id.id;
        max_ids[1] = start_object_id.id;
        iter = bpt_forward.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
}


PropertyPathBFSIterEnum::PropertyPathBFSIterEnum(BPlusTree<4>& type_from_to_edge,
                                                 BPlusTree<4>& to_type_from_edge,
                                                 Id start,
                                                 VarId end,
                                                 PathAutomaton automaton) :
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathBFSIterEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        first_next = true;
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            open.push(State(automaton.start,
                            start_object_id,
                            type_from_to_edge,
                            to_type_from_edge,
                            automaton)
            );
            visited.emplace(automaton.start, start_object_id);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.push(State(automaton.start,
                            start_object_id,
                            type_from_to_edge,
                            to_type_from_edge,
                            automaton)
            );
            visited.emplace(automaton.start, start_object_id);
        }
    }
}


bool PropertyPathBFSIterEnum::next() {
    // Check if first node is final
    if (first_next) {
        first_next = false;
        if (automaton.end.find(open.front().automaton_start_state) != automaton.end.end()) {
            first_next = false;
            parent_binding->add(end, open.front().start_object_id);
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.front();
        if (current_state.has_next(visited)) {
            bool is_final = automaton.end.find(current_state.reached_automaton_state) != automaton.end.end();
            open.push(State(current_state.reached_automaton_state,
                            current_state.reached_object_id,
                            type_from_to_edge,
                            to_type_from_edge,
                            automaton)
            );
            visited.insert(StateKey(current_state.reached_automaton_state, current_state.reached_object_id));

            if (is_final) {
                // set binding;
                parent_binding->add(end, current_state.reached_object_id);
                return true;
            }
        } else {
            open.pop();
        }
    }
    return false;
}


void PropertyPathBFSIterEnum::reset() {
    // Empty open and visited
    queue<State> empty;
    open.swap(empty);
    visited.clear();

    first_next = true;
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.push(State(automaton.start,
                        start_object_id,
                        type_from_to_edge,
                        to_type_from_edge,
                        automaton)
        );
        visited.emplace(automaton.start, ObjectId(start_object_id));

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.push(State(automaton.start,
                        start_object_id,
                        type_from_to_edge,
                        to_type_from_edge,
                        automaton)
        );
        visited.emplace(automaton.start, ObjectId(start_object_id));
    }
}


void PropertyPathBFSIterEnum::assign_nulls() { }


void PropertyPathBFSIterEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSIterEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
