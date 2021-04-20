#include "property_path_iddfs_check.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;


PropertyPathIDDFSCheck::PropertyPathIDDFSCheck(BPlusTree<4>& type_from_to_edge,
                                     BPlusTree<4>& to_type_from_edge,
                                     Id start,
                                     Id end,
                                     PathAutomaton automaton) :
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathIDDFSCheck::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    current_max_deep = 0;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            auto start_state = SearchState(automaton.start, start_object_id);
            open_next_step.push(make_pair(start_state, 0));
            visited.insert(start_state);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            auto start_state = SearchState(automaton.start, start_object_id);
            open_next_step.push(make_pair(start_state, 0));
            visited.insert(start_state);
        }

        // Set end_object_id
        if (std::holds_alternative<ObjectId>(end)) {
            end_object_id = std::get<ObjectId>(end);
        } else {
            auto end_var_id = std::get<VarId>(end);
            end_object_id = parent_binding[end_var_id];
        }

        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        // pos 0 and 1 will be set at next()
    }
}


bool PropertyPathIDDFSCheck::next() {
    while (open_next_step.size()) {
        visited.clear();
        open.swap(open_next_step);
        current_max_deep += max_deep_step;
        while (open.size() > 0) {
            auto current_pair = open.top();
            open.pop();
            auto current_state = current_pair.first;
            auto current_deep = current_pair.second;
            if (automaton.end.find(current_state.state) != automaton.end.end()
                && current_state.object_id == end_object_id )
            {
                stack<pair<SearchState, uint64_t>> empty;
                open.swap(empty);
                results_found++;
                return true;
            } else {
                unique_ptr<BptIter<4>> it;
                for (const auto& transition : automaton.transitions[current_state.state]) {
                    if (transition.inverse) {
                        min_ids[0] = current_state.object_id.id;
                        max_ids[0] = current_state.object_id.id;
                        min_ids[1] = transition.label.id;
                        max_ids[1] = transition.label.id;
                        it = to_type_from_edge.get_range(
                            Record<4>(min_ids),
                            Record<4>(max_ids)
                        );
                    } else {
                        min_ids[0] = transition.label.id;
                        max_ids[0] = transition.label.id;
                        min_ids[1] = current_state.object_id.id;
                        max_ids[1] = current_state.object_id.id;
                        it = type_from_to_edge.get_range(
                            Record<4>(min_ids),
                            Record<4>(max_ids)
                        );
                    }
                    bpt_searches++;
                    auto child_record = it->next();
                    auto open_pointer = current_deep < current_max_deep ? &open : &open_next_step;
                    while (child_record != nullptr) {
                        auto next_state = SearchState(transition.to, ObjectId(child_record->ids[2]));
                        if (visited.find(next_state) == visited.end()) {
                            visited.insert(next_state);
                            open_pointer->push(make_pair(next_state, current_deep + 1));
                        }
                        child_record = it->next();
                    }
                }
            }
        }
    }
    return false;
}


void PropertyPathIDDFSCheck::reset() {
    // Empty open and visited
    stack<pair<SearchState,uint64_t>> empty;
       stack<pair<SearchState,uint64_t>> empty_next;
    current_max_deep = 0;
    open.swap(empty);
    visited.clear();
    open_next_step.swap(empty_next);

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_state = SearchState(automaton.start, start_object_id);
        open_next_step.push(make_pair(start_state, 0));
        visited.insert(start_state);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        auto start_state = SearchState(automaton.start, start_object_id);
        open_next_step.push(make_pair(start_state, 0));
        visited.insert(start_state);
    }

    // Set end_object_id
    if (std::holds_alternative<ObjectId>(end)) {
        end_object_id = std::get<ObjectId>(end);
    } else {
        auto end_var_id = std::get<VarId>(end);
        end_object_id = (*parent_binding)[end_var_id];
    }
}


void PropertyPathIDDFSCheck::assign_nulls() { }


void PropertyPathIDDFSCheck::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathIDDFSCheck(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
