#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprTZ : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprTZ(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        if (expr_oid.get_type() != ObjectId::MASK_DATETIME)
            return ObjectId::get_null();

        if (expr_oid.id & (1ULL << 54)) {
            // Precision bit is set, return default value as timezone is not stored
            return Conversions::pack_string("Z");
        } else {
            // Precision bit is not set, timezone is stored normally
            uint64_t tz_sign =  (expr_oid.id & 0x0000'0000'0000'0800) >> 11;
            uint64_t tz_hour  = (expr_oid.id & 0x0000'0000'0000'07C0) >> 6;
            uint64_t tz_min   = (expr_oid.id & 0x0000'0000'0000'003F);
            if (tz_hour == 0 && tz_min == 0) {
                return Conversions::pack_string("Z");
            }
            else {
                std::stringstream ss;
                ss << std::setfill('0')
                   << ((tz_sign) ? '-' : '+')
                   << std::setw(2) << tz_hour << ':'
                   << std::setw(2) << tz_min;
                std::cout << "TZ: " << ss.str() << std::endl;
                return Conversions::pack_string(ss.str());
            }
        }
    }
};
