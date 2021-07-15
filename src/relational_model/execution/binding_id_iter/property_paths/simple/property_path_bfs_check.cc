#include "property_path_bfs_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/record.h"

using namespace std;

PropertyPathBFSCheck::PropertyPathBFSCheck(BPlusTree<1>& _nodes,
                                           BPlusTree<4>& _type_from_to_edge,
                                           BPlusTree<4>& _to_type_from_edge,
                                           VarId         _path_var,
                                           Id            _start,
                                           Id            _end,
                                           PathAutomaton _automaton) :
    nodes             (_nodes),
    type_from_to_edge (_type_from_to_edge),
    to_type_from_edge (_to_type_from_edge),
    path_var          (_path_var),
    start             (_start),
    end               (_end),
    automaton         (_automaton)
    { }


void PropertyPathBFSCheck::begin(BindingId& _parent_binding) {
    parent_binding = &_parent_binding;
    // Init start object id
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]);

    // Add to open and visited structures
    auto start_state = visited.emplace(automaton.get_start(),
                                        start_object_id,
                                        nullptr,
                                        true,
                                        ObjectId::get_null());

    open.push(start_state.first.operator->());

    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*parent_binding)[end_var_id];
    }
    is_first = true;
    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;
}


bool PropertyPathBFSCheck::next() {
    // Check if first node is end state
    if (is_first) {
        is_first = false;

        auto current_state = open.front();
        auto node_iter = nodes.get_range(Record<1>({current_state->object_id.id}),
                                         Record<1>({current_state->object_id.id}));
        // Return false if node does not exists in bd
        if (node_iter->next() == nullptr) {
            queue<const SearchState*> empty;
            open.swap(empty);
            return false;
        }
        if (automaton.start_is_final && (current_state->object_id == end_object_id)) {
            auto path_id = path_manager.set_path(current_state, path_var);
            parent_binding->add(path_var, path_id);
            queue<const SearchState*> empty;
            open.swap(empty);
            results_found++;
            return true;
        }
    }
    // BFS classic implementation
    while (open.size() > 0) {
        auto& current_state = open.front();
        // Expand state. Only visit nodes that automatons transitions indicates
        for (const auto& transition : automaton.transitions[current_state->state]) {
            set_iter(transition, current_state);

            // Explore matches nodes
            auto child_record = iter->next();
            while (child_record != nullptr) {
                auto next_state = SearchState(
                    transition.to,
                    ObjectId(child_record->ids[2]),
                    current_state,
                    transition.inverse,
                    transition.label);

                auto next_state_pointer = visited.insert(next_state);
                // Check if next_state was added to visited
                if (next_state_pointer.second) {
                    open.push(next_state_pointer.first.operator->());
                }

                // Check if next_state is final
                if (next_state.state == automaton.get_final_state()
                    && next_state.object_id == end_object_id)
                {
                    auto path_id = path_manager.set_path(next_state_pointer.first.operator->(),
                                                         path_var);
                    parent_binding->add(path_var, path_id);
                    queue<const SearchState*> empty;
                    open.swap(empty);
                    results_found++;
                    return true;
                }
                child_record = iter->next();
            }
        }
        // Pop to visit next state
        open.pop();
    }
    return false;
}


void PropertyPathBFSCheck::set_iter(const TransitionId& transition, const SearchState* current_state) {
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


void PropertyPathBFSCheck::reset() {
    // Empty structures
    queue<const SearchState*> empty;
    open.swap(empty);
    visited.clear();
    is_first = true;

    // Add start object id to structures
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]
    );

    auto start_state = visited.emplace(automaton.get_start(),
                                       start_object_id,
                                       nullptr,
                                       true,
                                       ObjectId::get_null());

    open.push(start_state.first.operator->());

    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*parent_binding)[end_var_id];
    }
}


void PropertyPathBFSCheck::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSCheck(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
