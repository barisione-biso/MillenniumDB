#ifndef BASE__COMPARISON_H_
#define BASE__COMPARISON_H_

#include <iostream>
#include <memory>

#include "base/binding/binding.h"
#include "base/graph/condition/condition.h"
#include "base/graph/condition/value_assign.h"
#include "base/graph/condition/value_assign_constant.h"
#include "base/graph/condition/value_assign_property.h"
#include "base/graph/condition/value_assign_variable.h"
#include "base/graph/value/value.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/common/value_visitor.h"

class Comparison : public Condition {
public:
    std::unique_ptr<ValueAssign> rhs;
    std::unique_ptr<ValueAssign> lhs;

    Comparison(query::ast::Statement const& statement) {
        // LHS
        if (statement.lhs.key) {
            lhs = std::make_unique<ValueAssignProperty>(statement.lhs.var, statement.lhs.key.get());
        } else {
            lhs = std::make_unique<ValueAssignVariable>(statement.lhs.var);
        }

        // RHS
        if (statement.rhs.type() == typeid(query::ast::SelectItem)) {
            auto casted_rhs = boost::get<query::ast::SelectItem>(statement.rhs);
            if (casted_rhs.key) {
                rhs = std::make_unique<ValueAssignProperty>(casted_rhs.var, casted_rhs.key.get());
            } else {
                rhs = std::make_unique<ValueAssignVariable>(casted_rhs.var);
            }
        } else {
            auto casted_rhs = boost::get<query::ast::Value>(statement.rhs);
            auto visitor = ValueVisitor();
            auto value = visitor(casted_rhs);
            rhs = std::make_unique<ValueAssignConstant>(move(value));
        }
     }

    virtual ~Comparison() = default;

    virtual bool compare(GraphObject& lhs, GraphObject& rhs) = 0;

    bool eval(Binding& binding) {
        auto left_value = lhs->get_value(binding);
        auto right_value = rhs->get_value(binding);

        if (left_value != nullptr) {
            return compare(*left_value, *right_value);
        } else {
            return false;
        }
    }

    ConditionType type() {
        return ConditionType::comparison;
    }

    void check_names(std::set<std::string>& var_names) {
        lhs->check_names(var_names);
        rhs->check_names(var_names);
    }
};

#endif // BASE__COMPARISON_H_
