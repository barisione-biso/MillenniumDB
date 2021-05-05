#include "property_path_a_star_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;
using namespace AStarIterEnum;

PropertyPathAStarIterEnum::PropertyPathAStarIterEnum(BPlusTree<4>& type_from_to_edge,
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


void PropertyPathAStarIterEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            open.emplace(
                automaton.start,
                start_object_id,
                automaton.distance_to_final[automaton.start]
            );
            visited.emplace(automaton.start, start_object_id);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.emplace(
                automaton.start,
                start_object_id,
                automaton.distance_to_final[automaton.start]
            );
            visited.emplace(automaton.start, start_object_id);
        }
        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        is_first = true;
        // pos 0 and 1 will be set at next()
    }
}


bool PropertyPathAStarIterEnum::next() {
    if (is_first) {
        is_first = false;
        if (automaton.start_is_final) {
            auto& current_state = open.top();
            visited.emplace(automaton.final_state, current_state.object_id);
            parent_binding->add(end, current_state.object_id);
            return true;
        }
    }
    while (open.size() > 0) {
        if (current_state_has_next()) {
            auto& current_state = open.top();
            open.emplace(reached_automaton_state, reached_object_id, current_state.priority);
            if (reached_automaton_state == automaton.final_state) {
                // set binding;
                parent_binding->add(end, reached_object_id);
                return true;
            }
        } else {
            open.pop();
        }
    }
    return false;
}

bool PropertyPathAStarIterEnum::current_state_has_next() {
    auto& current_state = open.top();
    auto state_transition = current_state.transition;
    auto state = current_state.state;
    if (current_state.iter == nullptr) {
        if (state_transition < automaton.transitions[state].size()) {
            set_iter(current_state);
            state_transition = 0;
        } else {
            return false;
        }
    }
    while (state_transition < automaton.transitions[state].size()) {
        auto& current_state = open.top();
        state_transition = current_state.transition;
        state = current_state.state;
        auto& transition = automaton.transitions[state][state_transition];
        auto child_record = current_state.iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto next_object_id = ObjectId(child_record->ids[2]);
            auto next_automaton_state = transition.to;
            auto next_state_key = SearchState(next_automaton_state, next_object_id);
            // Check child is not already visited
            if (visited.find(next_state_key) == visited.end()) {
                visited.insert(next_state_key);
                // Update next state settings
                reached_automaton_state = transition.to;
                reached_object_id = next_object_id;
                return true;
            }
            child_record = current_state.iter->next();
        }
        // Constructs new iter
        if (state_transition < automaton.transitions[state].size()) {
            set_iter(current_state);
            state_transition++;
        }
    }
    return false;
}


void PropertyPathAStarIterEnum::set_iter(const PriorityIterState&  current_state) {
    auto state              = current_state.state;
    auto object_id          = current_state.object_id;
    auto priority           = current_state.priority;
    auto current_transition = current_state.transition;

    auto new_state = PriorityIterState(state, object_id, priority);
    if (current_state.iter != nullptr) {
         new_state.transition = current_transition + 1;
    } else {
        new_state.transition = 0;
    }

    const auto& transition = automaton.transitions[state][new_state.transition];


    open.pop();

    if (transition.inverse) {
        min_ids[0] = object_id.id;
        max_ids[0] = object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        new_state.iter = to_type_from_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = object_id.id;
        max_ids[1] = object_id.id;
        new_state.iter = type_from_to_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
    open.push(move(new_state));
}


void PropertyPathAStarIterEnum::reset() {
    // Empty open and visited
    priority_queue<PriorityIterState> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_state = SearchState(automaton.start, start_object_id);
        open.emplace(
            automaton.start,
            start_object_id,
            automaton.distance_to_final[automaton.start]);
        visited.emplace(automaton.start, start_object_id);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        auto start_state = SearchState(automaton.start, start_object_id);
        open.emplace(
            automaton.start,
            start_object_id,
            automaton.distance_to_final[automaton.start]
        );
        visited.emplace(automaton.start, start_object_id);
    }
    is_first = true;
}


void PropertyPathAStarIterEnum::assign_nulls() { }


void PropertyPathAStarIterEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathAStarIterEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
