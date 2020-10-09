#ifndef RELATIONAL_MODEL__MERGE_JOIN_H_
#define RELATIONAL_MODEL__MERGE_JOIN_H_

#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"

#include <memory>

// This merge is valid only under these conditions:
// - lhs and rhs have one variable in common: `join_var`
// - lhs and rhs will be ordered by `join_var`
// - lhs and rhs won't give any duplicates of `join_var`

class MergeJoin : public BindingIdIter {
public:
    MergeJoin(std::unique_ptr<BindingIdIter> lhs,
              std::unique_ptr<BindingIdIter> rhs,
              VarId join_var);
    ~MergeJoin() = default;

    void analyze(int indent = 0) const override;
    BindingId* begin(BindingId& input) override;
    void reset() override;
    bool next() override;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    VarId join_var;

    BindingId* current_left = nullptr;
    BindingId* current_right = nullptr;

    std::unique_ptr<BindingId> my_binding;

    // statistics
    uint_fast32_t results_found = 0;

    void construct_binding();
};

#endif // RELATIONAL_MODEL__MERGE_JOIN_H_
