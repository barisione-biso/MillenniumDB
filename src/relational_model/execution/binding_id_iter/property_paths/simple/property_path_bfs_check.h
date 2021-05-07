#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_CHECK_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_CHECK_H_

#include <array>
#include <memory>
#include <unordered_set>
//#include <set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSCheck will determine if there exists a path between
2 nodes: `start` & `end` using clasical BFS algorithm to explore the database.
Use an automaton to only explore paths that match with the asked path
*/

class PropertyPathBFSCheck : public BindingIdIter {
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

    // Structs for BFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    std::queue<SearchState> open;
    bool is_first = false;  // True if is the first iteration
    std::unique_ptr<BptIter<4>> iter = nullptr;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    // Constructs iter according to transition
    void set_iter(const TransitionId& transition, const SearchState& current_state);

public:
    PropertyPathBFSCheck(BPlusTree<4>& type_from_to_edge,
                      BPlusTree<4>& to_type_from_edge,
                      Id start,
                      Id end,
                      PathAutomaton automaton);
    ~PropertyPathBFSCheck() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;


};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_BFS_CHECK_H_
