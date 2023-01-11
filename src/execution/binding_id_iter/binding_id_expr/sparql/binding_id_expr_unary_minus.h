#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprUnaryMinus : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprUnaryMinus(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_value = expr->eval(binding_id);

        uint64_t type = expr_value.id & ObjectId::TYPE_MASK;

        switch(type) {
            // TODO: Implement this
            // case ObjectId::MASK_NEGATIVE_INT:
            // case ObjectId::MASK_POSITIVE_INT:
            // case ObjectId::MASK_FLOAT:
            // case ObjectId::MASK_DECIMAL_INLINED:
            // case ObjectId::MASK_DECIMAL_EXTERN:
            default:
                return ObjectId::get_null();
        }
    }
};
