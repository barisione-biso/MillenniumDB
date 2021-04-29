#ifndef RELATIONAL_MODEL__PROPERTY_PATH_DFS_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_DFS_ITER_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <stack>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
//#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"



namespace DFSIterEnum {

struct State {
    const uint32_t state;
    const ObjectId object_id;
    uint32_t current_transition = 0;
    std::unique_ptr<BptIter<4>> iter = nullptr;

    State(uint32_t state, ObjectId object_id) :
        state     (state),
        object_id (object_id) { }

};

struct StateKey {
    uint32_t state;
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
};



class PropertyPathDFSIterEnum : public BindingIdIter {
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
     std::array<uint64_t, 4> min_ids;
     std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<DFSIterEnum::StateKey, DFSIterEnum::StateKeyHasher> visited;
    std::stack<DFSIterEnum::State> open;

    bool first_next = true;

    // std::unique_ptr<BptIter<4>> iter;
    uint32_t        reached_automaton_state = 0;
    ObjectId        reached_object_id;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    PropertyPathDFSIterEnum(BPlusTree<4>& type_from_to_edge,
                            BPlusTree<4>& to_type_from_edge,
                            Id start,
                            VarId end,
                            PathAutomaton automaton);
    ~PropertyPathDFSIterEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
    bool current_state_has_next(DFSIterEnum::State& current_state);
    void set_iter(DFSIterEnum::State& current_state);
};


#endif // RELATIONAL_MODEL__PROPERTY_PATH_DFS_ITER_ENUM_H_
