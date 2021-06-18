#include "property_path_bfs_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/record.h"

using namespace std;


PropertyPathBFSCheck::PropertyPathBFSCheck(
                                    BPlusTree<1>& nodes,
                                    BPlusTree<4>& type_from_to_edge,
                                    BPlusTree<4>& to_type_from_edge,
                                    VarId path_var,
                                    Id start,
                                    Id end,
                                    PathAutomaton automaton) :
    nodes             (nodes),
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    path_var          (path_var),
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

        auto current_state = open.front();
        auto node_iter = nodes.get_range(
            Record<1>({current_state.object_id.id}),
            Record<1>({current_state.object_id.id}));
        // Return false if node does not exists in bd
        if (node_iter->next() == nullptr) {
            queue<SearchState> empty;
            open.swap(empty);
            return false;
        }
        if (automaton.start_is_final && (current_state.object_id == end_object_id)) {
            auto path_id = path_manager.set_path(
                visited.find(current_state).operator->(), path_var);
            parent_binding->add(path_var, path_id);
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
        for (const auto& transition : automaton.transitions[current_state.state]) {
            set_iter(transition, current_state);

            // Explore matches nodes
            auto child_record = iter->next();
            while (child_record != nullptr) {
                auto next_state = SearchState(
                    transition.to,
                    ObjectId(child_record->ids[2]),
                    visited.find(current_state).operator->(),
                    transition.inverse,
                    transition.label);

                // Add to visited and queue
                if (visited.find(next_state) == visited.end()) {
                    open.push(next_state);
                    visited.insert(next_state);
                }

                // Check if next_state is final
                if (next_state.state == automaton.get_final_state()
                    && next_state.object_id == end_object_id )
                {
                    auto path_id = path_manager.set_path(
                        visited.find(next_state).operator->(), path_var);
                    parent_binding->add(path_var, path_id);
                    queue<SearchState> empty;
                    open.swap(empty);
                    results_found++;
                    return true;
                }
                child_record = iter->next();
            }
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
    is_first = true;

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
