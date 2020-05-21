#ifndef RELATIONAL_MODEL__MERGE_JOIN_H_
#define RELATIONAL_MODEL__MERGE_JOIN_H_

#include "base/ids/var_id.h"
#include "relational_model/binding/binding_id_iter.h"

#include <memory>
#include <vector>

class MergeJoin : public BindingIdIter {
public:
    MergeJoin(std::unique_ptr<BindingIdIter> lhs,
              std::unique_ptr<BindingIdIter> rhs,
              std::vector<VarId> join_vars);
    ~MergeJoin() = default;

    void begin(BindingId& input);
    void reset(BindingId& input);
    BindingId* next();

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    std::vector<VarId> join_vars;

    BindingId* current_left;
    BindingId* current_right;

    std::unique_ptr<BindingId> my_binding;

    void construct_binding(BindingId& lhs, BindingId& rhs);
};

#endif // RELATIONAL_MODEL__MERGE_JOIN_H_
