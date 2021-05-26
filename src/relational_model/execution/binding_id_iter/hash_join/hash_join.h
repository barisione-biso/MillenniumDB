#ifndef RELATIONAL_MODEL__HASH_JOIN_H_
#define RELATIONAL_MODEL__HASH_JOIN_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/hash_join/key_value_pair_hasher.h"
#include "storage/index/hash/key_value_hash/key_value_hash.h"
#include "storage/index/hash/hash_functions/hash_function_wrapper.h"
#include "storage/page.h"


class HashJoin : public BindingIdIter {
public:

    static constexpr uint_fast32_t MAX_SIZE_SMALL_HASH = Page::PAGE_SIZE*1024;

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

    KeyValueHash<ObjectId, ObjectId> lhs_hash;
    KeyValueHash<ObjectId, ObjectId> rhs_hash;

    bool enumerating_with_second_hash;
    bool enumerating_with_nested_loop;
    bool left_min;
    std::unordered_multimap<std::vector<ObjectId>,
                            std::vector<ObjectId>,
                            KeyValuePairHasher> small_hash;

    uint_fast32_t current_pos_left;   // for nested loop
    uint_fast32_t current_pos_right;
    uint_fast32_t current_bucket;
    std::unordered_multimap<std::vector<ObjectId>,
                            std::vector<ObjectId>,
                            KeyValuePairHasher>::iterator current_pair_iter;  // for small hash
    std::unordered_multimap<std::vector<ObjectId>,
                            std::vector<ObjectId>,
                            KeyValuePairHasher>::iterator end_range_iter;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
    std::pair<std::vector<ObjectId>, std::vector<ObjectId>> saved_pair;

    void assign_binding(const std::pair<std::vector<ObjectId>, std::vector<ObjectId>>& lhs_pair,
                        const std::pair<std::vector<ObjectId>, std::vector<ObjectId>>& rhs_pair);
};

#endif // RELATIONAL_MODEL__HASH_JOIN_H_
