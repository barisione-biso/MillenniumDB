#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_CHECK_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_CHECK_H_

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
PropertyPathBFSCheck will determine if there exists a path between
2 nodes: `start` & `end` using clasical BFS algorithm to explore the database.
Use an automaton to only explore paths that match with the asked path
*/

namespace BFSCheck {

struct CheckState {
    const uint32_t state;
    const ObjectId object_id;
    const CheckState* previous;

    CheckState(unsigned int state, ObjectId object_id, const CheckState* previous) :
        state      (state),
        object_id  (object_id),
        previous   (previous) { }

    ~CheckState() = default;

    bool operator<(const CheckState& other) const {
        if (state < other.state) {
            return true;
        } else if (other.state < state) {
            return false;
        } else {
            return object_id < other.object_id;
        }
    }

    bool operator==(const CheckState& other) const {
        return state == other.state && object_id.id == other.object_id.id;
    }
};
struct CheckStateHasher {
    std::size_t operator() (const CheckState& lhs) const {
      return lhs.state ^ lhs.object_id.id;
    }
};
};


class PropertyPathBFSCheck : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    QuadModel&    model;
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
    std::unordered_set<BFSCheck::CheckState, BFSCheck::CheckStateHasher> visited;
    std::queue<BFSCheck::CheckState> open;
    bool is_first = false;  // // True in the first call of next
    std::unique_ptr<BptIter<4>> iter = nullptr;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    // Constructs iter according to transition
    void set_iter(
        const TransitionId& transition,
        const BFSCheck::CheckState& current_state);

    // Print path to state
    void print_path(CheckState& state);

public:
    PropertyPathBFSCheck(QuadModel&   model,
                        BPlusTree<4>& type_from_to_edge,
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
