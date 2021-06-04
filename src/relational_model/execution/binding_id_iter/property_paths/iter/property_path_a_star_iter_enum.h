#ifndef RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_ITER_ENUM_H_

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
PropertyPathAStarIterEnum enumerates paths from or to a specifc node.
Uses an heuristic always extract the nearest automaton state to
final state.
The paths returned are optimal thanks to AStar optimal
property.

Precaution with the hit time due to multiple extract and push
to open for some atribute modification
*/


namespace AStarIterEnum {

struct PriorityIterState {
    uint32_t state;
    ObjectId object_id;
    uint32_t priority;
    uint32_t transition = 0;
    std::unique_ptr<BptIter<4>> iter = nullptr;

    PriorityIterState(uint32_t state, ObjectId object_id, uint32_t distance_to_end) :
        state       (state),
        object_id   (object_id),
        priority    (distance_to_end) { }


    bool operator<(const PriorityIterState& rhs) const noexcept {
        // TODO:
        if (priority == rhs.priority) {
            if (iter != nullptr && rhs.iter == nullptr) {
                return false;
            } return true;
        }
        return priority > rhs.priority;
    }
};
}


class PropertyPathAStarIterEnum : public BindingIdIter {
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
    std::priority_queue<AStarIterEnum::PriorityIterState> open;
    bool is_first = false;

    uint32_t        reached_automaton_state = 0;
    ObjectId        reached_object_id;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    bool current_state_has_next();
    void set_iter(); // modifies open.top()

public:
    PropertyPathAStarIterEnum(
                      BPlusTree<4>& type_from_to_edge,
                      BPlusTree<4>& to_type_from_edge,
                      VarId path_var,
                      Id start,
                      VarId end,
                      PathAutomaton automaton);
    ~PropertyPathAStarIterEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;

};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_A_STAR_ITER_ENUM_H_
