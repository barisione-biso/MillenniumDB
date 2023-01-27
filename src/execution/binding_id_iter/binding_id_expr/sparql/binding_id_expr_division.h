#pragma once

#include <memory>
// #include <cmath>

#include "base/exceptions.h"
#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprDivision : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> lhs;
    std::unique_ptr<BindingIdExpr> rhs;

    BindingIdExprDivision(std::unique_ptr<BindingIdExpr> lhs, std::unique_ptr<BindingIdExpr> rhs) :
        lhs(std::move(lhs)), rhs(std::move(rhs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto lhs_oid = lhs->eval(binding_id);
        auto rhs_oid = rhs->eval(binding_id);

        auto optype = Conversions::calculate_optype(lhs_oid, rhs_oid);

        switch (optype) {
        case Conversions::OPTYPE_INTEGER: {
            // In SPARQL-1.1 integer division is done using decimals, and the result is a decimal
            auto rhs = Conversions::to_decimal(rhs_oid);
            if (rhs == 0) {
                return ObjectId::get_null();
            }
            auto lhs = Conversions::to_decimal(lhs_oid);
            return Conversions::pack_decimal(lhs / rhs);
        }
        case Conversions::OPTYPE_DECIMAL: {
            auto rhs = Conversions::to_decimal(rhs_oid);
              if (rhs == 0) {
                return ObjectId::get_null();
            }
            auto lhs = Conversions::to_decimal(lhs_oid);
            return Conversions::pack_decimal(lhs / rhs);
        }
        case Conversions::OPTYPE_FLOAT: {
            // Do the conversion and division using doubles to minimize precision loss.
            auto lhs = Conversions::to_double(lhs_oid);
            auto rhs = Conversions::to_double(rhs_oid);
            // Division by zero, etc is handle by the floating point implementation.
            // SPARQL 1.1 follows IEEE 754-2008.
            return Conversions::pack_float(lhs / rhs);
        }
        case Conversions::OPTYPE_INVALID: {
            return ObjectId::get_null();
        }
        default:
            throw LogicException("This should never happen");
        }
    }
};
