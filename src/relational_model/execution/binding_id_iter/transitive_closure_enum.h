#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_ENUM_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_ENUM_H_

#include <array>
#include <memory>
#include <queue>
#include <set>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"


class TransitiveClosureEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    BPlusTree<4>& bpt;
    Id start;
    VarId end;
    ObjectId type;
    uint_fast32_t start_pos;
    uint_fast32_t type_pos;

    // Attributes determined in begin
    BindingId* parent_binding;
    ObjectId start_object_id;
    std::unique_ptr<BptIter<4>> it;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
    std::array<uint64_t, 4> min_ids;
    std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::set<ObjectId> visited;
    std::queue<ObjectId> open;

    // Current children node
    std::unique_ptr<Record<4>> child_record;

    // Needed for border case where the initial node references itself
    bool self_reference;

    // Statistics
    uint_fast32_t bpt_searches = 0;
    uint_fast32_t results_found = 0;

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
