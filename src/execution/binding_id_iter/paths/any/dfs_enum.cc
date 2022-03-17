#include "dfs_enum.h"

#include <cassert>

#include "base/ids/var_id.h"
#include "execution/binding_id_iter/paths/path_manager.h"
#include "storage/index/bplus_tree/bplus_tree.h"

using namespace std;
using namespace Paths::Any;

DFSIterEnum::DFSIterEnum(ThreadInfo*   thread_info,
                                 BPlusTree<1>& nodes,
                                 BPlusTree<4>& type_from_to_edge,
                                 BPlusTree<4>& to_type_from_edge,
                                 VarId         path_var,
                                 Id            start,
                                 VarId         end,
                                 PathAutomaton automaton) :
    thread_info       (thread_info),
    nodes             (nodes),
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    path_var          (path_var),
    start             (start),
    end               (end),
    automaton         (automaton) { }


void DFSIterEnum::begin(BindingId& _parent_binding) {
    parent_binding = &_parent_binding;
    first_next     = true;

    // Add inital state to queue
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ? std::get<ObjectId>(start)
                                                                     : (*parent_binding)[std::get<VarId>(start)]);

    open.emplace(automaton.get_start(), start_object_id);

    visited.emplace(automaton.get_start(), start_object_id, nullptr, true, ObjectId::get_null());

    min_ids[2] = 0;
    max_ids[2] = 0xFFFFFFFFFFFFFFFF;
    min_ids[3] = 0;
    max_ids[3] = 0xFFFFFFFFFFFFFFFF;
}


bool DFSIterEnum::next() {
    // Check if first node is final
    if (first_next) {
        first_next            = false;
        auto& current_state   = open.top();
        auto  start_node_iter = nodes.get_range(&thread_info->interruption_requested,
                                               Record<1>({ current_state.object_id.id }),
                                               Record<1>({ current_state.object_id.id }));

        // Return false if node does not exists in bd
        if (start_node_iter->next() == nullptr) {
            return false;
        }
        if (automaton.start_is_final) {
            auto current_key =
              Paths::AnyShortest::SearchState(automaton.get_final_state(), open.top().object_id, nullptr, true, ObjectId::get_null());

            auto path_id = path_manager.set_path(visited.insert(current_key).first.operator->(), path_var);

            parent_binding->add(path_var, path_id);
            parent_binding->add(end, open.top().object_id);
            results_found++;
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.top();
        auto  reached_state = current_state_has_next(current_state);
        if (reached_state != visited.end()) {
            open.emplace(reached_state->automaton_state, reached_state->node_id);
            if (reached_state->automaton_state == automaton.get_final_state()) {
                auto path_id = path_manager.set_path(reached_state.operator->(), path_var);
                // set binding;
                parent_binding->add(path_var, path_id);
                parent_binding->add(end, reached_state->node_id);
                results_found++;
                return true;
            }
        } else {
            open.pop();
        }
    }
    return false;
}


robin_hood::unordered_set<Paths::AnyShortest::SearchState, Paths::AnyShortest::SearchStateHasher>::iterator
  DFSIterEnum::current_state_has_next(DFSSearchState& state) {
    if (state.iter == nullptr) { // if is first time that State is explore
        state.current_transition = 0;
        // Check automaton has transitions
        if (state.current_transition >= automaton.transitions[state.state].size()) {
            return visited.end();
        }
        // Constructs iter
        set_iter(state);
    }
    // Iterate over automaton_start state transtions
    while (state.current_transition < automaton.transitions[state.state].size()) {
        auto& transition   = automaton.transitions[state.state][state.current_transition];
        auto  child_record = state.iter->next();
        // Iterate over next_childs
        while (child_record != nullptr) {
            auto current_key = Paths::AnyShortest::SearchState(state.state, state.object_id, nullptr, true, ObjectId::get_null());

            auto next_state_key = Paths::AnyShortest::SearchState(transition.to,
                                              ObjectId(child_record->ids[2]),
                                              visited.find(current_key).operator->(),
                                              transition.inverse,
                                              transition.type);

            // Check child is not already visited
            auto inserted_state = visited.insert(next_state_key);
            // Inserted_state.second = true if and only if state was inserted
            if (inserted_state.second) {
                // Returns pointer to state
                return inserted_state.first;
            }
            child_record = state.iter->next();
        }
        // Constructs new iter
        state.current_transition++;
        if (state.current_transition < automaton.transitions[state.state].size()) {
            set_iter(state);
        }
    }
    return visited.end();
}


void DFSIterEnum::set_iter(DFSSearchState& state) {
    // Gets current transition object from automaton
    const auto& transition = automaton.transitions[state.state][state.current_transition];
    // Gets iter from correct bpt with transition.inverse
    if (transition.inverse) {
        min_ids[0] = state.object_id.id;
        max_ids[0] = state.object_id.id;
        min_ids[1] = transition.type.id;
        max_ids[1] = transition.type.id;
        state.iter =
          to_type_from_edge.get_range(&thread_info->interruption_requested, Record<4>(min_ids), Record<4>(max_ids));
    } else {
        min_ids[0] = transition.type.id;
        max_ids[0] = transition.type.id;
        min_ids[1] = state.object_id.id;
        max_ids[1] = state.object_id.id;
        state.iter =
          type_from_to_edge.get_range(&thread_info->interruption_requested, Record<4>(min_ids), Record<4>(max_ids));
    }
    bpt_searches++;
}


void DFSIterEnum::reset() {
    // Empty open and visited
    stack<DFSSearchState> empty;
    open.swap(empty);
    visited.clear();
    first_next = true;

    // Add start object id to open and visited
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ? std::get<ObjectId>(start)
                                                                     : (*parent_binding)[std::get<VarId>(start)]);

    open.emplace(automaton.get_start(), start_object_id);

    visited.emplace(automaton.get_start(), start_object_id, nullptr, true, ObjectId::get_null());
}


void DFSIterEnum::assign_nulls() {
    parent_binding->add(end, ObjectId::get_null());
}


void DFSIterEnum::analyze(std::ostream& os, int indent) const {
    os << std::string(indent, ' ');
    os << "Paths::Any::DFSIterEnum(bpt_searches: " << bpt_searches << ", found: " << results_found << ")";
}
