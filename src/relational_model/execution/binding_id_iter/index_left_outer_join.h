#ifndef RELATIONAL_MODEL__INDEX_LEFT_OUTER__JOIN_H_
#define RELATIONAL_MODEL__INDEX_LEFT_OUTER__JOIN_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"

class IndexLeftOuterJoin : public BindingIdIter {
public:
    IndexLeftOuterJoin(std::size_t binding_size,
                  std::unique_ptr<BindingIdIter> lhs,
                  std::unique_ptr<BindingIdIter> rhs);
    ~IndexLeftOuterJoin() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;
    bool has_result;
    bool has_left;
    uint_fast32_t results_found = 0;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    BindingId* parent_binding;
};

#endif // RELATIONAL_MODEL__INDEX_LEFT_OUTER__JOIN_H_
