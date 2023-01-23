#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprUnaryPlus : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprUnaryPlus(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch(expr_oid.get_type()) {
            case ObjectId::MASK_NEGATIVE_INT:
            case ObjectId::MASK_POSITIVE_INT:
            case ObjectId::MASK_FLOAT:
            case ObjectId::MASK_DECIMAL_INLINED:
            case ObjectId::MASK_DECIMAL_EXTERN:
                return expr_oid;
            default:
                return ObjectId::get_null();
        }
    }
};
