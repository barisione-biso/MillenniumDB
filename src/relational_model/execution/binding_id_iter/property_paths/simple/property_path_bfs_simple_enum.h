#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_SIMPLE_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_SIMPLE_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"
#include "relational_model/models/quad_model/quad_model.h"
//#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSSimpleEnum enumerate all nodes that can be reached from start with
a specific path. Use classic implementation of BFS algorithm.
*/

namespace BFSSimple {

struct BFSState {
    const uint32_t state;
    const ObjectId object_id;
    const BFSState* previous;

    BFSState(unsigned int state, ObjectId object_id, const BFSState* previous) :
        state      (state),
        object_id  (object_id),
        previous   (previous) { }

    ~BFSState() = default;

    bool operator<(const BFSState& other) const {
        if (state < other.state) {
            return true;
        } else if (other.state < state) {
            return false;
        } else {
            return object_id < other.object_id;
        }
    }

    bool operator==(const BFSState& other) const {
        return state == other.state && object_id.id == other.object_id.id;
    }
};


struct BFSStateHasher {
    std::size_t operator() (const BFSState& lhs) const {
      return lhs.state ^ lhs.object_id.id;
    }
};
}

class PropertyPathBFSSimpleEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor

    QuadModel&    model;

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
    std::unordered_set<BFSSimple::BFSState, BFSSimple::BFSStateHasher> visited;
    std::queue<BFSSimple::BFSState> open;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    // Constructs iter according to transition
    std::unique_ptr<BptIter<4>>  set_iter(
        const TransitionId& transition,
        const BFSSimple::BFSState& current_state);

    void print_path(BFSSimple::BFSState& state);

public:
    PropertyPathBFSSimpleEnum(
                      QuadModel&    model,
                      BPlusTree<4>& type_from_to_edge,
                      BPlusTree<4>& to_type_from_edge,
                      Id start,
                      VarId end,
                      PathAutomaton automaton);
    ~PropertyPathBFSSimpleEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_BFS_SIMPLE_ENUM_H_
