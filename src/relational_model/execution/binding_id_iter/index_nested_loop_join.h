#ifndef RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
#define RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_

#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"

#include <memory>
#include <vector>

class IndexNestedLoopJoin : public BindingIdIter {
public:
    IndexNestedLoopJoin(std::unique_ptr<BindingIdIter> lhs, std::unique_ptr<BindingIdIter> rhs);
    ~IndexNestedLoopJoin() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& input) override;
    void reset(BindingId& input) override;
    BindingId* next() override;


private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;

    BindingId* current_left;
    BindingId* current_right;

    std::vector<VarId> vars;
    std::unique_ptr<BindingId> my_binding;

    void construct_binding();
};

#endif // RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
