#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
//#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSCheck  returns nodes that can be reached by 'start' or reach to 'end' by a
path that automaton object describes.
  * Explores graph using BFS algorithm
*/

struct State;
struct StateKey;
struct StateKeyHasher;

struct State {
    // Start settings
    uint32_t  automaton_start_state; // TODO: MAKE CONST
    const ObjectId  start_object_id;

    BPlusTree<4>& bpt_forward; // TODO: tratar de eliminar
    BPlusTree<4>& bpt_inverse; // TODO: tratar de eliminar

    PathAutomaton& automaton;  // TODO: tratar de eliminar

    uint32_t        current_transtion = 0;

    // Next settings
    uint32_t        reached_automaton_state = 0;
    ObjectId        reached_object_id;


    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;
    std::unique_ptr<BptIter<4>> iter;

    State(uint32_t automaton_state, ObjectId start_object_id, BPlusTree<4>& bpt_forward, BPlusTree<4>& bpt_inverse, PathAutomaton& automaton) :
        automaton_start_state   (automaton_state),
        start_object_id         (start_object_id),
        bpt_forward             (bpt_forward),
        bpt_inverse             (bpt_inverse),
        automaton               (automaton),
        reached_object_id       (ObjectId::get_null()) {

            min_ids[2] = 0;
            max_ids[2] = 0xFFFFFFFFFFFFFFFF;
            min_ids[3] = 0;
            max_ids[3] = 0xFFFFFFFFFFFFFFFF;

        }
    // If has a next state, update next settings and return true
    bool has_next(std::unordered_set<StateKey, StateKeyHasher>& visited);

    // Set iter with current_transition and automaton_start_state attr
    void set_iter();
};


struct StateKey {
    uint32_t state; // TODO: Make const
    const ObjectId object_id;

    StateKey(uint32_t state, ObjectId object_id) :
        state     (state),
        object_id (object_id) { }

    bool operator==(const StateKey& other) const {
        return state == other.state && object_id == other.object_id;
    }
};


struct StateKeyHasher {
    std::size_t operator() (const StateKey& lhs) const {
        return lhs.state ^ lhs.object_id.id;
    }
};


class PropertyPathBFSIterEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor

    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward

    Id start;
    VarId end;
    PathAutomaton automaton;

    // Attributes determined in begin
    BindingId* parent_binding;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
    // std::array<uint64_t, 4> min_ids;
    // std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<StateKey, StateKeyHasher> visited;
    std::queue<State> open;
    bool first_next = true;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    PropertyPathBFSIterEnum(BPlusTree<4>& type_from_to_edge,
                            BPlusTree<4>& to_type_from_edge,
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
