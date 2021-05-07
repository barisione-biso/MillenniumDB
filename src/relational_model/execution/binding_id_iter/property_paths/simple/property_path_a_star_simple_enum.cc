#include "property_path_a_star_simple_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathAStarSimpleEnum::PropertyPathAStarSimpleEnum(BPlusTree<4>& type_from_to_edge,
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


void PropertyPathAStarSimpleEnum::begin(BindingId& parent_binding, bool parent_has_next) {
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


bool PropertyPathAStarSimpleEnum::next() {
    // Check if first node is final
    if (is_first) {
        is_first = false;
        if (automaton.start_is_final) {
            auto& current_state = open.top();
            visited.emplace(automaton.final_state, current_state.object_id);
            parent_binding->add(end, current_state.object_id);
            return true;
        }
    }
    // A Star classic implementation
    while (open.size() > 0) {
        // Extract pointer allow asign new value to current_state
        auto current_state = &open.top();
        // Expand node. Explore reachable nodes with automaton transitions
        while (current_state->transition < automaton.transitions[current_state->state].size()) {
            const auto& transition = automaton.transitions[current_state->state][current_state->transition];
            // Constructs iter with current automaton transition
            set_iter(transition, current_state);

            auto child_record = iter->next();
            bool find_next_state = false;  // True when a new state is added to open

            // Iterate over reachable nodes
            while (child_record != nullptr) {
                auto next_state = SearchState(transition.to, ObjectId(child_record->ids[2]));
                // Check if the node has been already visited
                if (visited.find(next_state) == visited.end()) {
                    open.emplace(
                        next_state.state,
                        next_state.object_id,
                        automaton.distance_to_final[next_state.state]
                    );
                    visited.insert(next_state);
                    current_state = &open.top();

                    // Check if node is final
                    if (next_state.state == automaton.final_state) {
                        results_found++;
                        parent_binding->add(end, next_state.object_id);
                        return true;
                    }
                    // Stop iterate, because a new node has been added
                    current_state = &open.top(); // Updates current_state pointer
                    find_next_state = true;
                    break;
                }
                child_record = iter->next();
            }
            // Check if a new node has been added
            if (find_next_state) {
                break;
            }

            // Updates current_state with the same state, but transition increase by one to explore
            // a new transition
            auto new_state = PriorityState(
                current_state->state,
                current_state->object_id,
                current_state->distance
            );
            new_state.transition = current_state->transition + 1;
            open.pop(); // pops current state
            open.push(new_state);  // new_state is pushed to the top, same priority that current_state

            // Update current_state with new_state
            current_state = &open.top();
        }

        // Check if current_state has more transitions to expand. Pop if not
        if (current_state->transition >= automaton.transitions[current_state->state].size()) {
            open.pop();
        }

        current_state = &open.top();
    }
    return false;
}

void PropertyPathAStarSimpleEnum::set_iter(
    const TransitionId& transition,
    const PriorityState* current_state) {
    // Get iter from correct bpt_tree according to inverse attribute
    if (transition.inverse) {
        min_ids[0] = current_state->object_id.id;
        max_ids[0] = current_state->object_id.id;
        min_ids[1] = transition.label.id;
        max_ids[1] = transition.label.id;
        iter = to_type_from_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.label.id;
        max_ids[0] = transition.label.id;
        min_ids[1] = current_state->object_id.id;
        max_ids[1] = current_state->object_id.id;
        iter = type_from_to_edge.get_range(Record<4>(min_ids), Record<4>(max_ids));
    }
    bpt_searches++;
}



void PropertyPathAStarSimpleEnum::reset() {
    // Empty open and visited
    priority_queue<PriorityState> empty;
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


void PropertyPathAStarSimpleEnum::assign_nulls() { }


void PropertyPathAStarSimpleEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathAStarSimpleEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
