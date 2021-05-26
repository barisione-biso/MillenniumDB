#ifndef RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_2_H_
#define RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_2_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/hash_join/key_value_pair_hasher.h"
#include "storage/index/hash/key_value_hash/key_value_hash.h"


class HashJoinInMemory2 : public BindingIdIter {
public:
    HashJoinInMemory2 (std::unique_ptr<BindingIdIter> lhs,
             std::unique_ptr<BindingIdIter> rhs,
             std::vector<VarId> left_vars,
             std::vector<VarId> common_vars,
             std::vector<VarId> right_vars);
    ~HashJoinInMemory2 () = default;

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

    bool enumerating;
    KeyValueHash<ObjectId, ObjectId> lhs_hash;  // asume left is the smallest one (from execution plan)
    uint_fast32_t current_bucket;
    uint_fast32_t current_bucket_pos;
    //MultiMap::iterator current_pair_iter;
    //MultiMap::iterator end_range_iter;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
    std::pair<std::vector<ObjectId>, std::vector<ObjectId>>  saved_pair;

    void assign_binding(const std::pair<std::vector<ObjectId>, std::vector<ObjectId>> & lhs_pair,
                        const std::pair<std::vector<ObjectId>, std::vector<ObjectId>> & rhs_pair);
};

#endif // RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_2_H_