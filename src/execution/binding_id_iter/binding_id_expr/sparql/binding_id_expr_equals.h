#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprEquals : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> lhs;
    std::unique_ptr<BindingIdExpr> rhs;

    BindingIdExprEquals(std::unique_ptr<BindingIdExpr> lhs, std::unique_ptr<BindingIdExpr> rhs) :
        lhs(std::move(lhs)), rhs(std::move(rhs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto lhs_oid = lhs->eval(binding_id);
        auto rhs_oid = rhs->eval(binding_id);

        if (lhs_oid.is_null() || rhs_oid.is_null()) {
            // Nulls are not equal to anything, including other nulls.
            return ObjectId::get_null();
        } else if (lhs_oid == rhs_oid) {
            // Fast path for the case where the two objects are the same
            return ObjectId(ObjectId::BOOL_TRUE);
        }
        // Otherwise, we need to do a more expensive comparison.
        // Numeric types can be compared between each other.
        if (lhs_oid.is_numeric() && rhs_oid.is_numeric()) {
            auto optype = Conversions::calculate_optype(lhs_oid, rhs_oid);
            switch (optype) {
            case Conversions::OPTYPE_INTEGER: {
                auto lhs = Conversions::to_integer(lhs_oid);
                auto rhs = Conversions::to_integer(rhs_oid);
                return ObjectId(ObjectId::MASK_BOOL | (lhs == rhs));
            }
            case Conversions::OPTYPE_DECIMAL: {
                auto lhs = Conversions::to_decimal(lhs_oid);
                auto rhs = Conversions::to_decimal(rhs_oid);
                return ObjectId(ObjectId::MASK_BOOL | (lhs == rhs));
            }
            case Conversions::OPTYPE_FLOAT: {
                auto lhs = Conversions::to_float(lhs_oid);
                auto rhs = Conversions::to_float(rhs_oid);
                return ObjectId(ObjectId::MASK_BOOL | (lhs == rhs));
            }
            case Conversions::OPTYPE_INVALID: {
                return ObjectId::get_null();
            }
            default:
                throw std::runtime_error("This should never happen.");
            }
        }
        // Otherwise, we need to check if the two objects are the same type.
        if (lhs_oid.get_type() == rhs_oid.get_type()) {
            return ObjectId(ObjectId::MASK_BOOL | (lhs_oid.get_value() == rhs_oid.get_value()));
        } else {
            return ObjectId::get_null();
        }

    }
};
