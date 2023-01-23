#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprSameTerm : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> lhs;
    std::unique_ptr<BindingIdExpr> rhs;

    BindingIdExprSameTerm(std::unique_ptr<BindingIdExpr> lhs, std::unique_ptr<BindingIdExpr> rhs) :
        lhs(std::move(lhs)), rhs(std::move(rhs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto lhs_oid = lhs->eval(binding_id);
        auto rhs_oid = rhs->eval(binding_id);

        // Unlike RDFterm-equal (=), sameTerm can be used to test for 
        // non-equivalent typed literals with unsupported datatypes.
        if (lhs_oid.is_null() || rhs_oid.is_null()) {
            // Nulls are not equal to anything, including other nulls.
            return ObjectId::get_null();
        } else {
            return ObjectId(ObjectId::MASK_BOOL | (lhs_oid == rhs_oid));
        }
    }
};
