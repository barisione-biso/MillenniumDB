#pragma once

#include <cassert>
#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprUnaryMinus : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprUnaryMinus(std::unique_ptr<BindingIdExpr> expr) : expr(std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch (expr_oid.get_type()) {
        case ObjectId::MASK_NEGATIVE_INT:
            return ObjectId(((~expr_oid.id) & ObjectId::VALUE_MASK) | ObjectId::MASK_POSITIVE_INT);
        case ObjectId::MASK_POSITIVE_INT:
            return ObjectId(((~expr_oid.id) & ObjectId::VALUE_MASK) | ObjectId::MASK_NEGATIVE_INT);
        case ObjectId::MASK_FLOAT:
            return ObjectId(expr_oid.id ^ Conversions::FLOAT_SIGN_MASK);
        case ObjectId::MASK_DECIMAL_INLINED:
            return ObjectId(expr_oid.id ^ Conversions::DECIMAL_SIGN_MASK);
        case ObjectId::MASK_DECIMAL_EXTERN: {
            auto d = Conversions::unpack_decimal_extern(expr_oid);
            return Conversions::pack_decimal(-d);
        }
        default:
            return ObjectId::get_null();
        }
    }
};
