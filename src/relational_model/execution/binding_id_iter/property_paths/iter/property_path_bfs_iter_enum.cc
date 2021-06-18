#include "property_path_bfs_iter_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;


PropertyPathBFSIterEnum::PropertyPathBFSIterEnum(
                                                BPlusTree<1>& nodes,
                                                BPlusTree<4>& type_from_to_edge,
                                                BPlusTree<4>& to_type_from_edge,
                                                VarId path_var,
                                                Id start,
                                                VarId end,
                                                PathAutomaton automaton) :
    nodes             (nodes),
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    path_var          (path_var),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathBFSIterEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            open.emplace(
                automaton.get_start(),
                start_object_id,
                nullptr,
                true,
                ObjectId(ObjectId::NULL_OBJECT_ID));
            visited.emplace(
                automaton.get_start(),
                start_object_id,
                nullptr,
                true,
                ObjectId(ObjectId::NULL_OBJECT_ID));
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.emplace(
                automaton.get_start(),
                start_object_id,
                nullptr,
                true,
                ObjectId(ObjectId::NULL_OBJECT_ID));
            visited.emplace(
                automaton.get_start(),
                start_object_id,
                nullptr,
                true,
                ObjectId(ObjectId::NULL_OBJECT_ID));
        }
        first_next = true;
        iter = nullptr;
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

        auto current_state = open.front();
        auto node_iter = nodes.get_range(
            Record<1>({current_state.object_id.id}),
            Record<1>({current_state.object_id.id}));
        // Return false if node does not exists in bd
        if (node_iter->next() == nullptr) {
            open.pop();
            return false;
        }

        if (automaton.start_is_final) {
            auto reached_key = SearchState(
                    automaton.get_final_state(),
                    open.front().object_id,
                    nullptr,
                    true,
                    ObjectId(ObjectId::NULL_OBJECT_ID));
            auto path_id = path_manager.set_path(
                visited.insert(reached_key).first.operator->(), path_var);
            parent_binding->add(path_var, path_id);
            parent_binding->add(end, open.front().object_id);
            results_found++;
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.front();
        auto state_reached = current_state_has_next(current_state);
        // If has next state then state_reached does not point to visited.end()
        if (state_reached != visited.end()) {
            open.emplace(
                reached_automaton_state,
                reached_object_id,
                nullptr,
                true,
                ObjectId(ObjectId::NULL_OBJECT_ID));
            if (reached_automaton_state == automaton.get_final_state()) {
                // set binding;
                auto path_id = path_manager.set_path(state_reached.operator->(), path_var);
                parent_binding->add(path_var, path_id);
                parent_binding->add(end, reached_object_id);
                results_found++;
                return true;
            }
        } else {
            iter = nullptr;
            open.pop();
        }
    }
    return false;
}


std::unordered_set<SearchState, SearchStateHasher>::iterator PropertyPathBFSIterEnum::current_state_has_next(const SearchState&  current_state) {
    if (iter == nullptr) { // if is first time that State is explore
        current_transition = 0;
        // Check automaton state has transitions
        if (current_transition >= automaton.transitions[current_state.state].size()) {
            return visited.end();
        }
        // Constructs iter
        set_iter(current_state);
    }
    // Iterate over automaton_start state transtions
    while (current_transition < automaton.transitions[current_state.state].size()) {
        auto& transition = automaton.transitions[current_state.state][current_transition];
        auto child_record = iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto next_object_id = ObjectId(child_record->ids[2]);
            auto next_automaton_state = transition.to;
            auto next_state_key = SearchState(
                next_automaton_state,
                next_object_id,
                visited.find(current_state).operator->(),
                transition.inverse,
                transition.label
                );
            // Check child is not already visited
            auto inserted_state = visited.insert(next_state_key);
            // Inserted_state.second = true if only if state was inserted
            if (inserted_state.second) {
                // Update next state settings
                reached_automaton_state = transition.to;
                reached_object_id = next_object_id;
                // Return pointer to state in visited
                return inserted_state.first;
            }
            child_record = iter->next();
        }
        // Constructs new iter
        current_transition++;
        if (current_transition < automaton.transitions[current_state.state].size()) {
            set_iter(current_state);
        }
    }
    return visited.end();
}


void PropertyPathBFSIterEnum::set_iter(const SearchState& current_state) {
    // Gets current transition object from automaton
    const auto& transition = automaton.transitions[current_state.state][current_transition];
    // Gets iter from correct bpt with transition.inverse
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


void PropertyPathBFSIterEnum::reset() {
    // Empty open and visited
    queue<SearchState> empty;
    open.swap(empty);
    visited.clear();

    first_next = true;
    iter = nullptr;
    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.emplace(
            automaton.get_start(),
            start_object_id,
            nullptr,
            true,
            ObjectId(ObjectId::NULL_OBJECT_ID));
        visited.emplace(
            automaton.get_start(),
            start_object_id,
            nullptr,
            true,
            ObjectId(ObjectId::NULL_OBJECT_ID));

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.emplace(
            automaton.get_start(),
            start_object_id,
            nullptr,
            true,
            ObjectId(ObjectId::NULL_OBJECT_ID));
        visited.emplace(
            automaton.get_start(),
            start_object_id,
            nullptr,
            true,
            ObjectId(ObjectId::NULL_OBJECT_ID));
    }
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
