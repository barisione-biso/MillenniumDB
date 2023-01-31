#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprIsIri : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprIsIri(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch(expr_oid.get_type()) {
            case ObjectId::MASK_IRI_INLINED:
            case ObjectId::MASK_IRI_EXTERN:
                return ObjectId(ObjectId::BOOL_TRUE);
            case ObjectId::MASK_NULL:
                return ObjectId::get_null();
            default:
                return ObjectId(ObjectId::BOOL_FALSE);
        }
    }
};
