#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/property_paths/path_automaton.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSIterEnum enumerates paths from or to a specifc node,
using BFS algorithm, thus paths returned are optimal.

Precaution with the exponencial memory usage of open due to
ramification of BFS
*/


class PropertyPathBFSIterEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    ThreadInfo*   thread_info;
    BPlusTree<1>& nodes;
    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward
    VarId         path_var;
    Id            start;
    VarId         end;
    PathAutomaton automaton;

    // Attributes determined in begin
    BindingId* parent_binding;
    bool first_next = true;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
     std::array<uint64_t, 4> min_ids;
     std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    // open stores a pointer to a SearchState stored in visited
    // that allows to avoid use visited.find to get a pointer and
    // use the state extracted of the open directly.
    std::queue<const SearchState*> open;

    // Stores the children of state in expansion
    std::unique_ptr<BptIter<4>> iter;
    // The index of the transition that set_iter method uses to
    // construct iter attribute.
    uint32_t current_transition = 0;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    std::unordered_set<SearchState, SearchStateHasher>::iterator
        current_state_has_next(const SearchState* current_state);

    // Set iter attribute that give all states that connects with
    // current_state with label of a specific transition
    void set_iter(const SearchState* current_state);

public:
    PropertyPathBFSIterEnum(ThreadInfo*   thread_info,
                            BPlusTree<1>& nodes,
                            BPlusTree<4>& type_from_to_edge,
                            BPlusTree<4>& to_type_from_edge,
                            VarId path_var,
                            Id start,
                            VarId end,
                            PathAutomaton automaton);
    ~PropertyPathBFSIterEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;

};


#endif // RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
