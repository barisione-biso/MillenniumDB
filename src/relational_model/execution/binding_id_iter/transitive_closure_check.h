#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_CHECK_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_CHECK_H_

#include <array>
#include <memory>
#include <set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/* TransitiveClosureCheck will determine if there exists a path between 2 nodes: `start` & `end`
 * following connections with type `type`.
 * If `start` or `end` are VarId, it assumes they will be propertly assignated in the binding
 * before `next()` is called
 */
class TransitiveClosureCheck : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    BPlusTree<4>& bpt;
    Id start;
    Id end;
    ObjectId type;
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
    std::set<ObjectId> visited;
    std::queue<ObjectId> open;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    TransitiveClosureCheck(std::size_t binding_size, BPlusTree<4>& bpt, Id start, Id end, ObjectId type,
                           uint_fast32_t start_pos, uint_fast32_t type_pos);
    ~TransitiveClosureCheck() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__TRANSITIVE_CLOSURE_CHECK_H_
