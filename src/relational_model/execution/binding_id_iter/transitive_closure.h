#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_

#include <array>
#include <memory>
#include <set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"


class TransitiveClosure : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    BPlusTree<4>& bpt;
    std::unique_ptr<BptIter<4>> it;
    Id start;
    Id end;
    ObjectId type;
    uint_fast32_t start_pos;
    uint_fast32_t type_pos;
    ObjectId end_object_id;
    BindingId* parent_binding;

    // BFS
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;
    std::set<ObjectId> visited; // Visited nodes
    std::queue<ObjectId> open;  // Expanded nodes

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    TransitiveClosure(std::size_t binding_size, BPlusTree<4>& bpt, Id start, Id end, ObjectId type,
                      uint_fast32_t start_pos, uint_fast32_t type_pos);
    ~TransitiveClosure() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    bool next() override;
    void assign_nulls() override;
};

#endif // RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_
