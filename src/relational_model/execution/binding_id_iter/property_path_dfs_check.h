#ifndef RELATIONAL_MODEL__PROPERTY_PATH_DFS_CHECK_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_DFS_CHECK_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <stack>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
#include "base/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathDFSCheck will determine if there exists a path between 2 nodes: `start` & `end`
  * A path is validate with automaton
  * Explores graph using DFS algorithm
  * The search is only for the first valid path
*/

class PropertyPathDFSCheck : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward
    Id start;
    Id end;
    PathAutomaton automaton;

    // Attributes determined in begin
    ObjectId end_object_id;
    BindingId* parent_binding;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;

    // Structs for DFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    std::stack<SearchState> open;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    PropertyPathDFSCheck(BPlusTree<4>& type_from_to_edge,
                      BPlusTree<4>& to_type_from_edge,
                      Id start,
                      Id end,
                      PathAutomaton automaton);
    ~PropertyPathDFSCheck() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_DFS_CHECK_H_
