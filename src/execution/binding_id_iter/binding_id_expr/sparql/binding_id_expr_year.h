#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprYear : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprYear(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (expr_oid.get_type() != ObjectId::MASK_DATETIME)
            return ObjectId::get_null();

        int64_t year;
        if (expr_oid.id & (1ULL << 54)) {
            // Precision bit is set, year is is stored in the lower 38 bits
            year = (expr_oid.id & 0x003F'FFFF'FFFF'FFFF);
        } else {
            // Precision bit is not set, year is stored normally
            year = (expr_oid.id & 0x000F'FFC0'0000'0000) >> 38;
        }
        if (expr_oid.id & (1ULL << 55)) {
            // Sign bit is set, year is negative
            year *= -1;
        }
        return Conversions::pack_int(year);
    }
};
