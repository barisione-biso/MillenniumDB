#ifndef RELATIONAL_MODEL__HASH_JOIN_H_
#define RELATIONAL_MODEL__HASH_JOIN_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "storage/index/multi_map/multi_map.h"
#include "storage/index/hash_table/murmur3.h"
#include "relational_model/execution/binding_id_iter/hash_join/multipair_hasher.h"

class HashJoin : public BindingIdIter {
public:

    static constexpr uint_fast32_t MAX_SIZE_SMALL_HASH = 4096*1024;//4096*1024;

    HashJoin(std::unique_ptr<BindingIdIter> lhs,
             std::unique_ptr<BindingIdIter> rhs,
             std::vector<VarId> left_vars,
             std::vector<VarId> common_vars,
             std::vector<VarId> right_vars);
    ~HashJoin() = default;

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

    MultiMap lhs_hash;
    MultiMap rhs_hash;

    bool enumerating_with_second_hash;
    bool enumerating_with_nested_loop;
    bool left_min;
    SmallMultiMap small_hash;

    uint_fast32_t current_pos_left;   // inside bucket
    uint_fast32_t current_pos_right;  // inside bucket
    uint_fast32_t current_bucket;
    SmallMultiMap::iterator current_pair_iter;
    SmallMultiMap::iterator end_range_iter;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
    MultiPair saved_pair;

    void assign_binding(const MultiPair& lhs_pair, const MultiPair& rhs_pair);
};

#endif // RELATIONAL_MODEL__HASH_JOIN_H_
