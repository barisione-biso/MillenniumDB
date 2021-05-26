#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSCheck  returns nodes that can be reached by 'start' or reach to 'end' by a
path that automaton object describes.
  * Explores graph using BFS algorithm
*/


class PropertyPathBFSIterEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward
    VarId         path_var;

    Id start;
    VarId end;
    PathAutomaton automaton;

    // Attributes determined in begin
    BindingId* parent_binding;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
     std::array<uint64_t, 4> min_ids;
     std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    std::queue<SearchState> open;
    bool first_next = true;

    std::unique_ptr<BptIter<4>> iter;
    uint32_t        reached_automaton_state = 0;
    uint32_t        current_transition = 0;
    ObjectId        reached_object_id;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    bool current_state_has_next(const SearchState& current_state);
    void set_iter(const SearchState& current_state);

public:
    PropertyPathBFSIterEnum(
                            BPlusTree<4>& type_from_to_edge,
                            BPlusTree<4>& to_type_from_edge,
                            VarId path_var,
                            Id start,
                            VarId end,
                            PathAutomaton automaton);
    ~PropertyPathBFSIterEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;

};


#endif // RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
