#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprNot : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprNot(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_value = expr->eval(binding_id);
        
        ObjectId expr_bool = Conversions::to_boolean(expr_value);

        if (expr_bool.id == ObjectId::NULL_OBJECT_ID) {
            return expr_bool;
        } else {
            return ObjectId(expr_bool.id | 1UL);
        }
    }
};
