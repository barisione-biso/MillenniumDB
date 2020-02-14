#ifndef BASE__EQUALS_H_
#define BASE__EQUALS_H_

#include "base/binding/binding.h"
#include "base/parser/grammar/ast.h"
#include "base/parser/grammar/value_visitor.h"
#include "base/graph/condition/condition.h"
#include "base/graph/condition/value_assign.h"
#include "base/graph/condition/value_assign_constant.h"
#include "base/graph/condition/value_assign_variable.h"
#include "base/graph/value/value.h"

#include <iostream>
#include <memory>

class Equals : public Condition {
public:
    std::unique_ptr<ValueAssign> rhs;
    std::unique_ptr<ValueAssign> lhs;

    Equals(ast::Statement const& statement) {
        lhs = std::make_unique<ValueAssignVariable>(statement.lhs.variable, statement.lhs.key);

        if (statement.rhs.type() == typeid(ast::Element)) {
            auto casted_rhs = boost::get<ast::Element>(statement.rhs);
            rhs = std::make_unique<ValueAssignVariable>(casted_rhs.variable, casted_rhs.key);
        }
        else {
            auto casted_rhs = boost::get<ast::Value>(statement.rhs);
            auto visitor = ValueVisitor();
            auto value = visitor(casted_rhs);
            rhs = std::make_unique<ValueAssignConstant>(move(value));
        }
     }

    bool eval(Binding& binding) {

        auto left_value = lhs->get_value(binding);
        auto right_value = rhs->get_value(binding);

        if (left_value != nullptr) {
            return left_value == right_value;
        }
        else {
            return false;
        }
    }

    ConditionType type() {
        return ConditionType::comparison;
    }
};

#endif //BASE__EQUALS_H_
