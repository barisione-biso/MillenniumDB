#include "property_path_a_star_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/path_manager.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;
using namespace AStarIterEnum;

PropertyPathAStarIterEnum::PropertyPathAStarIterEnum(
                                    QuadModel&    model,
                                    BPlusTree<4>& type_from_to_edge,
                                    BPlusTree<4>& to_type_from_edge,
                                    Id start,
                                    VarId end,
                                    PathAutomaton automaton) :
    model             (model),
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
            visited.emplace(automaton.start, start_object_id, nullptr);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.emplace(
                automaton.start,
                start_object_id,
                automaton.distance_to_final[automaton.start]
            );
            visited.emplace(automaton.start, start_object_id, nullptr);
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
            visited.emplace(
                automaton.final_state,
                current_state.object_id,
                nullptr);
            auto current_key = SearchState(
                automaton.final_state,
                current_state.object_id,
                nullptr
            );
            // set binding;
            auto path_id = path_manager.set_path(visited.find(current_key).operator->(), 0);
            parent_binding->add(end, path_id);
            parent_binding->add(end, current_state.object_id);
            results_found++;
            return true;
        }
    }
    while (open.size() > 0) {
        if (current_state_has_next()) {
            auto& current_state = open.top();
            open.emplace(reached_automaton_state, reached_object_id, current_state.priority);
            if (reached_automaton_state == automaton.final_state) {
                // set binding;
                auto current_key = SearchState(
                    reached_automaton_state,
                    reached_object_id,
                    nullptr
                );

                auto path_id = path_manager.set_path(visited.find(current_key).operator->(), 0); // TODO:
                parent_binding->add(end, path_id);
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


bool PropertyPathAStarIterEnum::current_state_has_next() {
    auto current_state = &open.top();
    if (current_state->iter == nullptr) {
        if (current_state->transition < automaton.transitions[current_state->state].size()) {
            set_iter();
            current_state = &open.top(); // set_iter modifies open.top()
        } else {
            return false;
        }
    }

    while (current_state->transition < automaton.transitions[current_state->state].size()) {
        auto& transition = automaton.transitions[current_state->state][current_state->transition];
        auto child_record = current_state->iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto next_object_id = ObjectId(child_record->ids[2]);
            auto next_automaton_state = transition.to;
            auto current_key = SearchState(
                current_state->state, current_state->object_id, nullptr
            );
            auto next_state_key = SearchState(
                next_automaton_state,
                next_object_id,
                visited.find(current_key).operator->()
                );
            // Check child is not already visited
            if (visited.find(next_state_key) == visited.end()) {
                visited.insert(next_state_key);
                // Update next state settings
                reached_automaton_state = transition.to;
                reached_object_id = next_object_id;
                return true;
            }
            child_record = current_state->iter->next();
        }
        // Constructs new iter
        if (current_state->transition < automaton.transitions[current_state->state].size()) {
            set_iter();
            current_state = &open.top(); // set_iter modifies open.top()
        }
    }
    return false;
}


void PropertyPathAStarIterEnum::set_iter() {
    auto current_state = &open.top();
    PriorityIterState new_state(current_state->state, current_state->object_id, current_state->priority);
    if (current_state->iter != nullptr) {
        new_state.transition = current_state->transition + 1;
    } else {
        new_state.transition = 0;
    }

    const auto& transition = automaton.transitions[new_state.state][new_state.transition];
    if (transition.inverse) {
        min_ids[0] = new_state.object_id.id;
        max_ids[0] = new_state.object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        new_state.iter = to_type_from_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = new_state.object_id.id;
        max_ids[1] = new_state.object_id.id;
        new_state.iter = type_from_to_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
    bpt_searches++;
    open.pop();
    open.push(move(new_state));
}


void PropertyPathAStarIterEnum::reset() {
    // Empty open and visited
    priority_queue<PriorityIterState> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.emplace(
            automaton.start,
            start_object_id,
            automaton.distance_to_final[automaton.start]);
        visited.emplace(automaton.start, start_object_id, nullptr);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.emplace(
            automaton.start,
            start_object_id,
            automaton.distance_to_final[automaton.start]
        );
        visited.emplace(automaton.start, start_object_id, nullptr);
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


void PropertyPathAStarIterEnum::print_path(const SearchState& state) {
    auto current_state = &state;
    while (current_state != nullptr) {
        cout << model.get_graph_object(current_state->object_id) << "<=";
        current_state = const_cast<SearchState*>(current_state->previous);
    }
    cout << "\n";
}
