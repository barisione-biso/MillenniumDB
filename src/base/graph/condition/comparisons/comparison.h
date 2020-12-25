#ifndef BASE__COMPARISON_H_
#define BASE__COMPARISON_H_

#include <cassert>
#include <memory>
#include <set>

#include "base/binding/binding.h"
#include "base/graph/condition/condition.h"
#include "base/graph/condition/value_assign/value_assign.h"
#include "base/graph/condition/value_assign/value_assign_constant.h"
#include "base/graph/condition/value_assign/value_assign_variable.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/common/value_visitor.h"

class Comparison : public Condition {
public:
    std::unique_ptr<ValueAssign> lhs;
    std::unique_ptr<ValueAssign> rhs;

    Comparison(std::unique_ptr<ValueAssign> lhs, std::unique_ptr<ValueAssign> rhs) :
         lhs (std::move(lhs)),
         rhs (std::move(rhs)) { }

    virtual ~Comparison() = default;
    virtual bool compare(const GraphObject lhs, const GraphObject rhs) const = 0;

    ConditionType type() {
        return ConditionType::comparison;
    }

    bool eval(Binding& binding) {
        return compare(lhs->get_value(binding), rhs->get_value(binding));
    }
};

#endif // BASE__COMPARISON_H_
