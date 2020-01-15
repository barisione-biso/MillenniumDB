#ifndef RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
#define RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_

#include "base/var/var_id.h"
#include "relational_model/physical_plan/binding_id_iter.h"

#include <map>
#include <memory>
#include <vector>


class IndexNestedLoopJoin : public BindingIdIter {
public:
    IndexNestedLoopJoin(BindingIdIter& left, BindingIdIter& right);
    ~IndexNestedLoopJoin() = default;
    void init(BindingId& input);
    void reset(BindingId& input);
    BindingId* next();

private:
    void construct_binding(BindingId& lhs, BindingId& rhs);

    BindingIdIter& left;
    BindingIdIter& right;

    BindingId* current_left;
    BindingId* current_right;

    std::vector<VarId> vars;
    std::unique_ptr<BindingId> my_binding;
};

#endif //RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
