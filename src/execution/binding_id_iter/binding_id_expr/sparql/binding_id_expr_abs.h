#pragma once

#include <memory>
// #include <cmath>

#include "base/exceptions.h"
#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprAbs : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprAbs(std::unique_ptr<BindingIdExpr> expr) : expr(std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (!expr_oid.is_numeric()) {
            return ObjectId::get_null();
        }

        switch (expr_oid.get_type()) {
        case ObjectId::MASK_NEGATIVE_INT:
            return ObjectId(((~expr_oid.id) & ObjectId::VALUE_MASK) | ObjectId::MASK_POSITIVE_INT);
        case ObjectId::MASK_POSITIVE_INT:
            return expr_oid;
        case ObjectId::MASK_DECIMAL_INLINED:
            return ObjectId(expr_oid.id & (~Conversions::DECIMAL_SIGN_MASK));
        case ObjectId::MASK_DECIMAL_EXTERN: {
            auto d = Conversions::unpack_decimal_extern(expr_oid);
            d.sign = false;
            return Conversions::pack_decimal(d);
        }
        case ObjectId::MASK_FLOAT:
            return ObjectId(expr_oid.id & (~Conversions::FLOAT_SIGN_MASK));
        default:
            // This should never happen.
            throw LogicException("Incorrect type for Abs");
        }
    }
};
