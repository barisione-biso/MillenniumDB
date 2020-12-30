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
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
    bool has_result;
    bool has_left;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;
    // std::unique_ptr<BindingIdIter> basic_graph_pattern;
    // std::vector<std::unique_ptr<Le>> children;

    BindingId* current_left;
    BindingId* current_right;
};

#endif // RELATIONAL_MODEL__LEFT_OUTER__JOIN_H_
