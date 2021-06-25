#include "property_path_bfs_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathBFSIterEnum::PropertyPathBFSIterEnum(BPlusTree<1>& _nodes,
                                                 BPlusTree<4>& _type_from_to_edge,
                                                 BPlusTree<4>& _to_type_from_edge,
                                                 VarId         _path_var,
                                                 Id            _start,
                                                 VarId         _end,
                                                 PathAutomaton _automaton) :
    nodes             (_nodes),
    type_from_to_edge (_type_from_to_edge),
    to_type_from_edge (_to_type_from_edge),
    path_var          (_path_var),
    start             (_start),
    end               (_end),
    automaton         (_automaton)
    { }


void PropertyPathBFSIterEnum::begin(BindingId& _parent_binding, bool parent_has_next) {
    parent_binding = &_parent_binding;
    if (parent_has_next) {
        // First next = true allow to next method knows if is doing the first iteration
        first_next = true;

        iter = nullptr;
        // Add start object id to open and visited
        ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
            std::get<ObjectId>(start) :
            (*parent_binding)[std::get<VarId>(start)]);

        auto state_inserted = visited.emplace(automaton.get_start(),
                                              start_object_id,
                                              nullptr,
                                              true,
                                              ObjectId::get_null());

        open.push(state_inserted.first.operator->());

        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
    }
}


bool PropertyPathBFSIterEnum::next() {
    // Check if first node is final
    if (first_next) {
        first_next = false;

        const auto current_state = open.front();
        auto node_iter = nodes.get_range(Record<1>({current_state->object_id.id}),
                                         Record<1>({current_state->object_id.id}));
        // Return false if node does not exists in bd
        if (node_iter->next() == nullptr) {
            open.pop();
            return false;
        }

        if (automaton.start_is_final) {
            auto reached_key = SearchState(automaton.get_final_state(),
                                           current_state->object_id,
                                           nullptr,
                                           true,
                                           ObjectId::get_null());

            auto path_id = path_manager.set_path(visited.insert(reached_key).first.operator->(), path_var);
            parent_binding->add(path_var, path_id);
            parent_binding->add(end, current_state->object_id);
            results_found++;
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.front();
        auto state_reached = current_state_has_next(current_state);
        // If has next state then state_reached does not point to visited.end()
        if (state_reached != visited.end()) {
            open.push(state_reached.operator->());

            if (state_reached->state == automaton.get_final_state()) {
                // set binding;
                auto path_id = path_manager.set_path(state_reached.operator->(), path_var);
                parent_binding->add(path_var, path_id);
                parent_binding->add(end, state_reached->object_id);
                results_found++;
                return true;
            }
        } else {
            // Pop and visit next state
            iter = nullptr;
            open.pop();
        }
    }
    return false;
}


unordered_set<SearchState, SearchStateHasher>::iterator
    PropertyPathBFSIterEnum::current_state_has_next(const SearchState* current_state)
{
    if (iter == nullptr) { // if is first time that State is explore
        current_transition = 0;
        // Check automaton state has transitions
        if (current_transition >= automaton.transitions[current_state->state].size()) {
            return visited.end();
        }
        // Constructs iter
        set_iter(current_state);
    }
    // Iterate over automaton_start state transtions
    while (current_transition < automaton.transitions[current_state->state].size()) {
        auto& transition = automaton.transitions[current_state->state][current_transition];
        auto child_record = iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {

            auto next_state_key = SearchState(
                transition.to,
                ObjectId(child_record->ids[2]),
                current_state,
                transition.inverse,
                transition.label);

            auto inserted_state = visited.insert(next_state_key);
            // Inserted_state.second = true if state was inserted in visited
            if (inserted_state.second) {
                // Return pointer to state in visited
                return inserted_state.first;
            }
            child_record = iter->next();
        }
        // Constructs new iter
        current_transition++;
        if (current_transition < automaton.transitions[current_state->state].size()) {
            set_iter(current_state);
        }
    }
    return visited.end();
}


void PropertyPathBFSIterEnum::set_iter(const SearchState* current_state) {
    // Gets current transition object from automaton
    const auto& transition = automaton.transitions[current_state->state][current_transition];
    // Gets iter from correct bpt with transition.inverse
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


void PropertyPathBFSIterEnum::reset() {
    // Empty open and visited
    queue<const SearchState*> empty;
    open.swap(empty);
    visited.clear();
    first_next = true;
    iter = nullptr;

    // Add start object id to open and visited
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]);

    auto state_inserted = visited.emplace(automaton.get_start(),
                                          start_object_id,
                                          nullptr,
                                          true,
                                          ObjectId::get_null());

    open.push(state_inserted.first.operator->());
}


void PropertyPathBFSIterEnum::assign_nulls() {
    parent_binding->add(end, ObjectId::get_null());
}


void PropertyPathBFSIterEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSIterEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
