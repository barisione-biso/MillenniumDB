#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprSeconds : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprSeconds(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (expr_oid.get_type() != ObjectId::MASK_DATETIME)
            return ObjectId::get_null();

        if (expr_oid.id & (1ULL << 54)) {
            // Precision bit is set, return default value as seconds is not stored
            return Conversions::pack_int(0);
        } else {
            // Precision bit is not set, seconds is stored normally
            uint64_t seconds = (expr_oid.id & 0x0000'0000'0003'F000) >> 12;
            return Conversions::pack_int(seconds);
        }
    }
};
