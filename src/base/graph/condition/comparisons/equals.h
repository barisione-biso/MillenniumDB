#ifndef BASE__EQUALS_H_
#define BASE__EQUALS_H_

#include "base/parser/grammar/ast.h"
#include "base/parser/grammar/value_visitor.h"
#include "base/graph/condition/condition.h"
#include "base/graph/condition/value_assign.h"
#include "base/graph/value/value.h"
#include "base/binding/binding.h"

#include <iostream>
#include <memory>

class Equals : public Condition {
public:
    // std::string left_name;
    // std::string left_var;
    // std::string left_key;

    // // only one of the two is valid. If rhs != null, right_name should be an empty string.
    // std::unique_ptr<Value> rhs;
    // std::string right_name;
    // std::string right_var;
    // std::string right_key;
    std::unique_ptr<ValueAssign> rhs;
    std::unique_ptr<ValueAssign> lhs;

    Equals(ast::Statement const& statement) {
        // left_var = statement.lhs.variable;
        // left_key = statement.lhs.key;
        // left_name = left_var + "." + left_key;

        if (statement.rhs.type() == typeid(ast::Element)) {
            // rhs = nullptr;
            auto casted_rhs = boost::get<ast::Element>(statement.rhs);
            // right_var = right.variable;
            // right_key = right.variable;
            // right_name = right_var + "." + right_key;
        }
        else {
            auto casted_rhs = boost::get<ast::Value>(statement.rhs);
            auto visitor = ValueVisitor();
            auto a = visitor(casted_rhs);
        }

     }

    bool eval(Binding& binding) {
        // auto lhs = binding[left_name];
        // if (lhs == nullptr) {
        //     // TODO: make join? necesitaria
        //     binding.print();
        //     lhs = binding.try_extend(left_var, left_key);
        // }

        // if (rhs != nullptr) { // If the right element is a constant value
        //     return *lhs == *rhs;
        // }
        // else {
        //     auto right = binding[right_name];
        //     return *lhs == *right;
        // }
        auto left_value = lhs->get_value(binding);
        auto right_value = rhs->get_value(binding);

        if (left_value != nullptr) {
            return right_value == left_value;
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
