#include "property_path_bfs_simple_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/record.h"

using namespace std;

PropertyPathBFSSimpleEnum::PropertyPathBFSSimpleEnum(BPlusTree<1>& _nodes,
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


void PropertyPathBFSSimpleEnum::begin(BindingId& _parent_binding) {
    parent_binding = &_parent_binding;

    // Create start object id
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]);

    // Add start object to open and visited
    open.emplace(automaton.get_start(),
                    start_object_id,
                    nullptr,
                    true,
                    ObjectId::get_null());

    visited.emplace(automaton.get_start(),
                    start_object_id,
                    nullptr,
                    true,
                    ObjectId::get_null());

    is_first = true;
    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;
}


bool PropertyPathBFSSimpleEnum::next() {
    // BFS classic implementation
    while (open.size() > 0) {
        auto& current_state = open.front();
        // Expand state. Explore reachable nodes with automaton transitions
        for (const auto& transition : automaton.transitions[current_state.state]) {
            // Constructs iter with current automaton transition
            auto iter = set_iter(transition, current_state);
            auto child_record = iter->next();

            // Explore nodes
            while (child_record != nullptr) {
                auto next_state = SearchState(transition.to,
                                              ObjectId(child_record->ids[2]),
                                              visited.find(current_state).operator->(),
                                              transition.inverse,
                                              transition.label);

                // Check if this node has been already visited
                if (visited.find(next_state) == visited.end()) {
                    // Add to open and visited set
                    open.push(next_state);
                    visited.insert(next_state);
                }
                child_record = iter->next();
            }
        }
        if (is_first) {
            is_first = false;
            auto start_node_iter = nodes.get_range(Record<1>({current_state.object_id.id}),
                                                   Record<1>({current_state.object_id.id}));

            // Return false if node does not exists in bd
            if (start_node_iter->next() == nullptr) {
                open.pop();
                return false;
            }
            if (current_state.state == automaton.get_start() &&
                automaton.start_is_final)
            {
                results_found++;
                auto path_object_id = path_manager.set_path(visited.find(current_state).operator->(),
                                                            path_var);

                parent_binding->add(path_var, path_object_id);
                parent_binding->add(end, current_state.object_id);
                open.pop();  // Pop to visit next state
                return true;
            }
        }
        // Check if current state is final
        else if (current_state.state == automaton.get_final_state()) {
            results_found++;
            auto path_object_id = path_manager.set_path(visited.find(current_state).operator->(),
                                                        path_var);

            parent_binding->add(path_var, path_object_id);
            parent_binding->add(end, current_state.object_id);
            open.pop();  // Pop to visit next state
            return true;
        }
        open.pop();  // Pop to visit next state
    }
    return false;
}


unique_ptr<BptIter<4>>  PropertyPathBFSSimpleEnum::set_iter(
    const TransitionId& transition,
    const SearchState& current_state) {
    unique_ptr<BptIter<4>> iter = nullptr;
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
    return iter;
}


void PropertyPathBFSSimpleEnum::reset() {
    // Empty open and visited
    queue<SearchState> empty;
    open.swap(empty);
    visited.clear();
    is_first = true;

    // Add start object id to open and visited structures
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]);

    open.emplace(automaton.get_start(),
                 start_object_id,
                 nullptr,
                 true,
                 ObjectId::get_null());

    visited.emplace(automaton.get_start(),
                    start_object_id,
                    nullptr,
                    true,
                    ObjectId::get_null());
}

void PropertyPathBFSSimpleEnum::assign_nulls() {
    parent_binding->add(end, ObjectId::get_null());
}


void PropertyPathBFSSimpleEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSSimpleEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
