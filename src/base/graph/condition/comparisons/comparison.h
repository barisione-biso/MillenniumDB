#ifndef BASE__COMPARISON_H_
#define BASE__COMPARISON_H_

#include <iostream>
#include <memory>

#include "base/binding/binding.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/value_visitor.h"
#include "base/graph/condition/condition.h"
#include "base/graph/condition/value_assign.h"
#include "base/graph/condition/value_assign_constant.h"
#include "base/graph/condition/value_assign_property.h"
#include "base/graph/condition/value_assign_variable.h"
#include "base/graph/value/value.h"

class Comparison : public Condition {
public:
    std::unique_ptr<ValueAssign> rhs;
    std::unique_ptr<ValueAssign> lhs;

    Comparison(query_ast::Statement const& statement) {
        // LHS
        if (statement.lhs.type() == typeid(query_ast::Var)) {
            auto casted_lhs = boost::get<query_ast::Var>(statement.lhs);
            lhs = std::make_unique<ValueAssignVariable>(casted_lhs.name);
        } else if (statement.lhs.type() == typeid(query_ast::Element)) {
            auto casted_lhs = boost::get<query_ast::Element>(statement.lhs);
            lhs = std::make_unique<ValueAssignProperty>(casted_lhs.var.name, casted_lhs.key);
        }

        // RHS
        if (statement.rhs.type() == typeid(ast::Var)) {
            auto casted_rhs = boost::get<ast::Var>(statement.rhs);
            rhs = std::make_unique<ValueAssignVariable>(casted_rhs.name);
        } else if (statement.rhs.type() == typeid(query_ast::Element)) {
            auto casted_rhs = boost::get<query_ast::Element>(statement.rhs);
            rhs = std::make_unique<ValueAssignProperty>(casted_rhs.var.name, casted_rhs.key);
        } else {
            auto casted_rhs = boost::get<ast::Value>(statement.rhs);
            auto visitor = ValueVisitor();
            auto value = visitor(casted_rhs);
            rhs = std::make_unique<ValueAssignConstant>(move(value));
        }
     }

    virtual ~Comparison() { };

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

    void check_names(std::set<std::string>& node_names, std::set<std::string>& edge_names) {
        lhs->check_names(node_names, edge_names);
        rhs->check_names(node_names, edge_names);
    }
};

#endif // BASE__COMPARISON_H_
