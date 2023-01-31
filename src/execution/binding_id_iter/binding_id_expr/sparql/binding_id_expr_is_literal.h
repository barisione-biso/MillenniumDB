#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprIsLiteral : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprIsLiteral(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch(expr_oid.get_type()) {
            case ObjectId::MASK_STRING_INLINED:
            case ObjectId::MASK_STRING_EXTERN:
            case ObjectId::MASK_STRING_LANG_INLINED:
            case ObjectId::MASK_STRING_LANG_EXTERN:
            case ObjectId::MASK_STRING_DATATYPE_INLINED:
            case ObjectId::MASK_STRING_DATATYPE_EXTERN:
            case ObjectId::MASK_NEGATIVE_INT:
            case ObjectId::MASK_POSITIVE_INT:
            case ObjectId::MASK_FLOAT:
            case ObjectId::MASK_BOOL:
            case ObjectId::MASK_DATETIME:
            case ObjectId::MASK_DECIMAL_INLINED:
            case ObjectId::MASK_DECIMAL_EXTERN:
                return ObjectId(ObjectId::BOOL_TRUE);
            case ObjectId::MASK_NULL:
                return ObjectId::get_null();
            default:
                return ObjectId(ObjectId::BOOL_FALSE);
        }
    }
};
