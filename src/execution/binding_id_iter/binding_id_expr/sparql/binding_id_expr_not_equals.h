#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprNotEquals : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> lhs;
    std::unique_ptr<BindingIdExpr> rhs;

    BindingIdExprNotEquals(std::unique_ptr<BindingIdExpr> lhs, std::unique_ptr<BindingIdExpr> rhs) :
        lhs (std::move(lhs)),
        rhs (std::move(rhs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto lhs_value = lhs->eval(binding_id);
        auto rhs_value = rhs->eval(binding_id);

        if (lhs_value != rhs_value) {
            return ObjectId(ObjectId::BOOL_TRUE);
        } else {
            return ObjectId(ObjectId::BOOL_FALSE);
        }
    }
};
