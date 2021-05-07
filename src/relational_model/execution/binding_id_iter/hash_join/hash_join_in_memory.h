#ifndef RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_H_
#define RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/hash_join/multipair_hasher.h"


class HashJoinInMemory : public BindingIdIter {
public:
    HashJoinInMemory (std::unique_ptr<BindingIdIter> lhs,
             std::unique_ptr<BindingIdIter> rhs,
             std::vector<VarId> left_vars,
             std::vector<VarId> common_vars,
             std::vector<VarId> right_vars);
    ~HashJoinInMemory () = default;

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
    SmallMultiMap lhs_hash;  // asume left is the smallest one (from execution plan)
    SmallMultiMap::iterator current_pair_iter;
    SmallMultiMap::iterator end_range_iter;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
    MultiPair saved_pair;

    void assign_binding(const MultiPair& lhs_pair, const MultiPair& rhs_pair);
};

#endif // RELATIONAL_MODEL__HASH_JOIN_IN_MEMORY_H_
