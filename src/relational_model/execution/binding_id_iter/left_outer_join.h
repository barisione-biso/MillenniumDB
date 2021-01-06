#ifndef RELATIONAL_MODEL__LEFT_OUTER__JOIN_H_
#define RELATIONAL_MODEL__LEFT_OUTER__JOIN_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"

class LeftOuterJoin : public BindingIdIter {
public:
    LeftOuterJoin(std::size_t binding_size,
                  std::unique_ptr<BindingIdIter> lhs,
                  std::unique_ptr<BindingIdIter> rhs);
    ~LeftOuterJoin() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;
    bool has_result;
    bool has_left;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    BindingId* parent_binding;
};

#endif // RELATIONAL_MODEL__LEFT_OUTER__JOIN_H_
