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
        auto expr_oid = expr->eval(binding_id);
        
        ObjectId expr_bool = Conversions::to_boolean(expr_oid);

        switch (expr_bool.id) {
        case ObjectId::BOOL_TRUE:
            return ObjectId(ObjectId::BOOL_FALSE);
        case ObjectId::BOOL_FALSE:
            return ObjectId(ObjectId::BOOL_TRUE);
        default:
            return ObjectId::get_null();
        }
    }
};
