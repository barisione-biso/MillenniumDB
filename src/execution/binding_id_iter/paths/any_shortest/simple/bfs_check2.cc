/*
This is a similar implementation to BFSSimpleEnum.
The first difference is the end condition, which is now determined not only
by reaching an end state of the automaton, but also the target node, whose
object ID we store in end_object_id.
The second and biggest difference, is that we need not scan all the neighbours
of a node in order to find the result. Namely, the process can be describe as
follows:
- We are looking for a path from start_object_id to end_object_id
- (start_object_id,initState) initializes BFS (both visited and open)
- If start_object_id == end_object_id, and initState is also a final state
  we can return a result and the execution halts.
- Else, we iterate over the neighbours of start_object_id node according to
  the automaton.
- If at any point we detect end_object_id we can return.
Notice that here the results can be returned as soon as detected, since we
are simply checking whether the two nodes are connected by a path.
*/

#include "bfs_check2.h"

#include <cassert>

#include "base/ids/var_id.h"
#include "execution/binding_id_iter/paths/path_manager.h"
#include "storage/index/record.h"

using namespace std;
using namespace Paths::AnyShortest;

BFSCheck2::BFSCheck2(ThreadInfo*  thread_info,
                     VarId        path_var,
                     Id           start,
                     Id           end,
                     RPQAutomaton automaton,
                     std::unique_ptr<PathIndexProvider> provider) :
    thread_info       (thread_info),
    path_var          (path_var),
    start             (start),
    end               (end),
    automaton         (automaton),
    provider          (std::move(provider))  { }


void BFSCheck2::begin(BindingId& _parent_binding) {
    parent_binding = &_parent_binding;

    // Init start object id
    ObjectId start_object_id(std::holds_alternative<ObjectId>(start) ?
        std::get<ObjectId>(start) :
        (*parent_binding)[std::get<VarId>(start)]);

    end_object_id = (std::holds_alternative<ObjectId>(end)) ?
        std::get<ObjectId>(end) :
        (*parent_binding)[std::get<VarId>(end)];

    // Add to open and visited structures
    auto start_state = visited.emplace(automaton.get_start(),
                                       start_object_id,
                                       nullptr,
                                       true,
                                       ObjectId::get_null());

    open.push(start_state.first.operator->());

    is_first = true;
}


bool BFSCheck2::next() {
    // Check if first node is end state
    if (is_first) {
        is_first = false;

        auto current_state = open.front();

        if (!provider->node_exists(current_state->node_id.id)) {
            open.pop();
            return false;
        }

        // Return false if node does not exists in bd
        if (automaton.start_is_final && (current_state->node_id == end_object_id)) {
            // auto path_id = path_manager.set_path(current_state, path_var);
            // parent_binding->add(path_var, path_id);
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
        for (const auto& transition : automaton.from_to_connections[current_state->automaton_state]) {
            set_iter(transition, current_state);

            // Explore matches nodes
            while (iter->next()) {
                auto next_state = SearchState(
                    transition.to,
                    ObjectId(iter->get()),
                    current_state,
                    transition.inverse,
                    transition.type_id);

                auto next_state_pointer = visited.insert(next_state);
                // Check if next_state was added to visited
                if (next_state_pointer.second) {
                    open.push(next_state_pointer.first.operator->());
                }

                // Check if next_state is final
                if (next_state.automaton_state == automaton.get_final_state()
                    && next_state.node_id == end_object_id)
                {
                    // auto path_id = path_manager.set_path(next_state_pointer.first.operator->(),
                    //                                      path_var);
                    // parent_binding->add(path_var, path_id);
                    queue<const SearchState*> empty;
                    open.swap(empty);
                    results_found++;
                    return true;
                }
            }
        }
        // Pop to visit next state
        open.pop();
    }
    return false;
}


void BFSCheck2::set_iter(const Transition& transition, const SearchState* current_state) {
    // Get iterator from custom index
    iter = provider->get_iterator(transition.type_id.id, transition.inverse, current_state->node_id.id);
}


void BFSCheck2::reset() {
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


void BFSCheck2::analyze(std::ostream& os, int indent) const {
    os << std::string(indent, ' ');
    os << "Paths::AnyShortest::BFSCheck2(bpt_searches: " << bpt_searches
       << ", found: " << results_found <<")";
}