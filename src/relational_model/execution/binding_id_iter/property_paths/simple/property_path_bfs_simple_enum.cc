#include "property_path_bfs_simple_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathBFSSimpleEnum::PropertyPathBFSSimpleEnum(
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


void PropertyPathBFSSimpleEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            open.emplace(automaton.start, start_object_id, nullptr);
            visited.emplace(automaton.start, start_object_id, nullptr);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            open.emplace(automaton.start, start_object_id, nullptr);
            visited.emplace(automaton.start, start_object_id, nullptr);
        }
        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        // pos 0 and 1 will be set at next()
    }
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
                auto next_state = SearchState(
                    transition.to,
                    ObjectId(child_record->ids[2]),
                    visited.find(current_state).operator->()
                );
                // Check if this node has been already visited
                if (visited.find(next_state) == visited.end()) {
                    // Add to open and visited set
                    open.push(next_state);
                    visited.insert(next_state);
                }
                child_record = iter->next();
            }
        }
        // Check if current state is final
        if (current_state.state == automaton.final_state ||
           (current_state.state == automaton.start && automaton.start_is_final))
        {
            results_found++;
            print_path(current_state);
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

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        open.emplace(automaton.start, start_object_id, nullptr);
        visited.emplace(automaton.start, start_object_id, nullptr);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        open.emplace(automaton.start, start_object_id, nullptr);
        visited.emplace(automaton.start, start_object_id, nullptr);
    }
}


void PropertyPathBFSSimpleEnum::assign_nulls() { }


void PropertyPathBFSSimpleEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSSimpleEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}

void PropertyPathBFSSimpleEnum::print_path(SearchState& state) {
    auto current_state = &state;
    while (current_state != nullptr) {
        cout << model.get_graph_object(current_state->object_id) << "<=";
        current_state = const_cast<SearchState*>(current_state->previous);
    }
    cout << "\n";
}
