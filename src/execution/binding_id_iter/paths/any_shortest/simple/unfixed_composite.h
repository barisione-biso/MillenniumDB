#pragma once

#include <array>
#include <memory>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/thread/thread_info.h"
#include "execution/binding_id_iter/paths/any_shortest/iter/bfs_iter_enum.h"
#include "parser/query/paths/automaton/rpq_automaton.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "third_party/robin_hood/robin_hood.h"

namespace Paths { namespace AnyShortest {
/*
UnfixedComposite removes the problem that when committing to expand a
transition of the automaton, it may obtain a non-optimal path. If all states
are expanded at the start, the cost is too high to pay, because all the paths
are expanded and finding among all those the first becomes expensive.

UnfixedComposite assumes that the path cannot be empty (initial state is not
final) so we start by looking at the transitions from the initial state to find
start nodes that make sense. It then does a search on that node using the
BFSIterEnum class, starting from the found node.
*/
class UnfixedComposite : public BindingIdIter {
private:
    // Attributes determined in the constructor
    ThreadInfo*  thread_info;
    VarId        path_var;
    VarId        start;
    VarId        end;
    RPQAutomaton automaton;

    // Attributes determined in begin
    BindingId* parent_binding;

    std::unique_ptr<BFSIterEnum> path_enum;

    // Structs to handle the fixed a node as start
    uint32_t current_start_transition = 0;

    std::unique_ptr<BptIter<4>> start_iter;

    std::unique_ptr<Record<4>> current_start_record = nullptr;

    // Only to remember the start nodes, in order to not repeat it
    robin_hood::unordered_node_set<uint64_t> visited;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    UnfixedComposite(ThreadInfo*  thread_info,
                     VarId        path_var,
                     VarId        start,
                     VarId        end,
                     RPQAutomaton automaton);

    void analyze(std::ostream& os, int indent = 0) const override;
    void begin(BindingId& parent_binding) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};
}} // namespace Paths::AllShortest
