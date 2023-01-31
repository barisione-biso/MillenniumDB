#pragma once

#include <memory>
#include <cmath>

#include "base/exceptions.h"
#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprFloor : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprFloor(std::unique_ptr<BindingIdExpr> expr) : expr(std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (!expr_oid.is_numeric()) {
            return ObjectId::get_null();
        }

        switch (expr_oid.get_type()) {
        case ObjectId::MASK_NEGATIVE_INT: {
            auto n = Conversions::unpack_negative_int(expr_oid);
            return ObjectId(Conversions::pack_int(std::floor(n)));
        }
        case ObjectId::MASK_POSITIVE_INT: {
            auto n = Conversions::unpack_positive_int(expr_oid);
            return ObjectId(Conversions::pack_int(std::floor(n)));
        }
        case ObjectId::MASK_DECIMAL_INLINED: {
            auto n = Conversions::unpack_decimal_inlined(expr_oid);
            return ObjectId(Conversions::pack_decimal(n.floor()));
        }
        case ObjectId::MASK_DECIMAL_EXTERN: {
            auto n = Conversions::unpack_decimal_extern(expr_oid);
            return ObjectId(Conversions::pack_decimal(n.floor()));
        }
        case ObjectId::MASK_FLOAT: {
            auto n = Conversions::unpack_float(expr_oid);
            return ObjectId(Conversions::pack_float(std::floor(n)));
        }
        default:
            // This should never happen.
            throw LogicException("Incorrect type for Floor");
        }
    }
};
