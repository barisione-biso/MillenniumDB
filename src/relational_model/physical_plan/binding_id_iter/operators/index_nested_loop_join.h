#ifndef RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
#define RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_

#include "base/var/var_id.h"
#include "relational_model/physical_plan/binding_id_iter/binding_id_iter.h"

#include <map>
#include <memory>
#include <vector>


class IndexNestedLoopJoin : public BindingIdIter {
public:
    IndexNestedLoopJoin(unique_ptr<BindingIdIter> left, unique_ptr<BindingIdIter> right);
    ~IndexNestedLoopJoin() = default;
    void init(std::shared_ptr<BindingId> input);
    void reset(std::shared_ptr<BindingId> input);
    std::unique_ptr<BindingId> next();

private:
    std::unique_ptr<BindingId> construct_binding(BindingId& lhs, BindingId& rhs);

    unique_ptr<BindingIdIter> left;
    unique_ptr<BindingIdIter> right;

    std::shared_ptr<BindingId> current_left;
    std::unique_ptr<BindingId> current_right;

    std::vector<VarId> vars;
};

#endif //RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
