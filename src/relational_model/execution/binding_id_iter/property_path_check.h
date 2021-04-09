#ifndef RELATIONAL_MODEL__PROPERTY_PATH_CHECK_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_CHECK_H_

#include <array>
#include <memory>
#include <set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

struct Pair {
    unsigned int state;
    ObjectId object_id;

    bool operator<(Pair& other) {
        if (state < other.state) {
            return true;
        } else {
            return object_id < other.object_id;
        }
    }
};


/* PropertyPathCheck will determine if there exists a path between 2 nodes: `start` & `end`
 * following connections with type `type`.
 * If `start` or `end` are VarId, it assumes they will be propertly assignated in the binding
 * before `next()` is called
 */
class PropertyPathCheck : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    BPlusTree<4>& bpt;
    Id start;
    Id end;
    PathAutomaton automaton;
    uint_fast32_t start_pos;
    uint_fast32_t type_pos;

    // Attributes determined in begin
    ObjectId end_object_id;
    BindingId* parent_binding;
    std::unique_ptr<BptIter<4>> it;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::set<Pair> visited;
    std::queue<Pair> open;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    PropertyPathCheck(BPlusTree<4>& bpt, Id start, Id end,
                        uint_fast32_t start_pos, uint_fast32_t type_pos, PathAutomaton automaton);
    ~PropertyPathCheck() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_CHECK_H_
