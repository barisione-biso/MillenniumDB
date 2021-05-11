#ifndef RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_SIMPLE_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_SIMPLE_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathAStarSimpleEnum enumerates all nodes that can be reached by start following
a specific path. Uses Greddy strategy to explore nodes, but the implementation
guarantees optimal state exploration order, for this reason consider that this operator
uses AStar implementation
*/


struct PriorityState {
    uint32_t state;
    ObjectId object_id;

    // Indicates the shortest distances from state to automaton final_state
    uint32_t distance;

    uint32_t transition = 0;

    PriorityState(uint32_t state, ObjectId object_id, uint32_t distance_to_end) :
        state       (state),
        object_id   (object_id),
        distance    (distance_to_end) { }

    // operator < defines the priority order of states
    bool operator<(const PriorityState& rhs) const noexcept {
        // If rhs > current_state, then rhs will be expand first.
        // Use greedy strategy to expand nodes with a lower distance.
        // Automaton structure guarantees optimal solution with a greddy strategy
        return distance > rhs.distance;
    }
};



class PropertyPathAStarSimpleEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor

    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward

    Id start;
    VarId end;
    PathAutomaton automaton;
    bool is_first = false; // True in the first next call


    // Attributes determined in begin
    BindingId* parent_binding;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    std::priority_queue<PriorityState> open;
    std::unique_ptr<BptIter<4>> iter = nullptr;


    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    // Constructs iter with the transition label
    void set_iter(const TransitionId& transition, const PriorityState& current_state);

public:
    PropertyPathAStarSimpleEnum(BPlusTree<4>& type_from_to_edge,
                      BPlusTree<4>& to_type_from_edge,
                      Id start,
                      VarId end,
                      PathAutomaton automaton);
    ~PropertyPathAStarSimpleEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_SIMPLE_ENUM_H_
