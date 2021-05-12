#include "property_path_dfs_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;
using namespace DFSIterEnum;


PropertyPathDFSIterEnum::PropertyPathDFSIterEnum(BPlusTree<4>& type_from_to_edge,
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


void PropertyPathDFSIterEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            open.emplace(automaton.start, start_object_id);
            visited.emplace(automaton.start, start_object_id);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.emplace(automaton.start, start_object_id);
            visited.emplace(automaton.start, start_object_id);
        }
        first_next = true;
        //iter = nullptr;
        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
    }
}


bool PropertyPathDFSIterEnum::next() {
    // Check if first node is final
    if (first_next) {
        first_next = false;
        if (automaton.start_is_final) {
            visited.emplace(automaton.final_state, open.top().object_id);
            parent_binding->add(end, open.top().object_id);
            results_found++;
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.top();
        if (current_state_has_next(current_state)) {
            open.emplace(reached_automaton_state, reached_object_id);
            if (reached_automaton_state == automaton.final_state) {
                // set binding;
                parent_binding->add(end, reached_object_id);
                results_found++;
                return true;
            }
        } else {
            open.pop();
        }
    }
    return false;
}


bool PropertyPathDFSIterEnum::current_state_has_next(State&  state) {
    if (state.iter == nullptr) { // if is first time that State is explore
        state.current_transition = 0;
        // Check automaton has transitions
        if (state.current_transition >= automaton.transitions[state.state].size()) {
            return false;
        }
        // Constructs iter
        set_iter(state);
    }
    // Iterate over automaton_start state transtions
    while (state.current_transition < automaton.transitions[state.state].size()) {
        auto& transition = automaton.transitions[state.state][state.current_transition];
        auto child_record = state.iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto next_object_id = ObjectId(child_record->ids[2]);
            auto next_automaton_state = transition.to;
            auto next_state_key = StateKey(next_automaton_state, next_object_id);
            // Check child is not already visited
            if (visited.find(next_state_key) == visited.end()) {
                visited.insert(next_state_key);
                // Update next state settings
                reached_automaton_state = transition.to;
                reached_object_id = next_object_id;
                return true;
            }
            child_record = state.iter->next();
        }
        // Constructs new iter
        state.current_transition++;
        if (state.current_transition < automaton.transitions[state.state].size()) {
            set_iter(state);
        }
    }
    return false;
}


void PropertyPathDFSIterEnum::set_iter(State& state) {
    // Gets current transition object from automaton
    const auto& transition = automaton.transitions[state.state][state.current_transition];
    // Gets iter from correct bpt with transition.inverse
    if (transition.inverse) {
        min_ids[0] = state.object_id.id;
        max_ids[0] = state.object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        state.iter = to_type_from_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = state.object_id.id;
        max_ids[1] = state.object_id.id;
        state.iter = type_from_to_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
    bpt_searches++;
}


void PropertyPathDFSIterEnum::reset() {
    // Empty open and visited
    stack<State> empty;
    open.swap(empty);
    visited.clear();

    first_next = true;
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.emplace(automaton.start, start_object_id);
        visited.emplace(automaton.start, ObjectId(start_object_id));

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.emplace(automaton.start, start_object_id);
        visited.emplace(automaton.start, ObjectId(start_object_id));
    }
}


void PropertyPathDFSIterEnum::assign_nulls() {
    parent_binding->add(end, ObjectId::get_null());
}


void PropertyPathDFSIterEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathDFSIterEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
