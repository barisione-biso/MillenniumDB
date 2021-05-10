#ifndef RELATIONAL_MODEL__LEFT_OUTER_HASH_JOIN_H_
#define RELATIONAL_MODEL__LEFT_OUTER_HASH_JOIN_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "storage/index/hash/key_value_hash/key_value_hash.h"
#include "storage/index/hash/murmur3.h"
#include "relational_model/execution/binding_id_iter/hash_join/key_value_pair_hasher.h"


class LeftOuterHashJoin : public BindingIdIter {
public:

    static constexpr uint_fast32_t MAX_SIZE_SMALL_HASH = 4096*1024;//4096*1024;

    LeftOuterHashJoin(std::unique_ptr<BindingIdIter> lhs,
             std::unique_ptr<BindingIdIter> rhs,
             std::vector<VarId> left_vars,
             std::vector<VarId> common_vars,
             std::vector<VarId> right_vars);
    ~LeftOuterHashJoin() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    std::vector<VarId> left_vars;
    std::vector<VarId> common_vars;
    std::vector<VarId> right_vars;

    BindingId* parent_binding;

    KeyValueHash lhs_hash;
    KeyValueHash rhs_hash;

    bool enumerating_with_second_hash;
    bool enumerating_with_nested_loop;
    //bool left_min;
    bool left_assigned;
    SmallMultiMap small_hash;  // using only for right

    uint_fast32_t current_pos_left;   // inside bucket
    uint_fast32_t current_pos_right;  // inside bucket
    uint_fast32_t current_bucket;
    SmallMultiMap::iterator current_pair_iter;
    SmallMultiMap::iterator end_range_iter;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
    KeyValuePair saved_pair;

    void assign_binding(const KeyValuePair& lhs_pair, const KeyValuePair& rhs_pair);
    void assign_binding_nulls(const KeyValuePair& lhs_pair);
};

#endif // RELATIONAL_MODEL__LEFT_OUTER_HASH_JOIN_H_