#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprIsNumeric : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprIsNumeric(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (expr_oid.is_null())
            return ObjectId::get_null();
        else
            return ObjectId(ObjectId::MASK_BOOL | expr_oid.is_numeric());
    }
};
