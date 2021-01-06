#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_ENUM_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_ENUM_H_

#include <array>
#include <memory>
#include <set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"


class TransitiveClosureEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    BPlusTree<4>& bpt;
    std::unique_ptr<BptIter<4>> it;

    BindingId* parent_binding;
    Id start;
    VarId end;
    ObjectId type;
    uint_fast32_t start_pos;
    uint_fast32_t type_pos;
    ObjectId end_object_id;

    // BFS
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;
    std::set<ObjectId> visited; // Visited nodes
    std::queue<ObjectId> open;  // Expanded nodes
    std::unique_ptr<Record<4>> child_record; // Current children node
    bool self_reference; // Needed for border case where the initial node references itself

    // Statistics
    uint_fast32_t outer_while_count = 0;
    uint_fast32_t inner_while_count = 0;
    uint_fast32_t bpt_searches = 0;

public:
    TransitiveClosureEnum(std::size_t binding_size, BPlusTree<4>& bpt, Id start, VarId end, ObjectId type,
                      uint_fast32_t start_pos, uint_fast32_t type_pos);
    ~TransitiveClosureEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    bool next() override;
    void assign_nulls() override;
};

#endif // RELATIONAL_MODEL__TRANSITIVE_CLOSURE_ENUM_H_
