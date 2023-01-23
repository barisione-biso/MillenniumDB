#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprIf : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr_cond;
    std::unique_ptr<BindingIdExpr> expr_then;
    std::unique_ptr<BindingIdExpr> expr_else;

    BindingIdExprIf(std::unique_ptr<BindingIdExpr> expr_cond,
                    std::unique_ptr<BindingIdExpr> expr_then,
                    std::unique_ptr<BindingIdExpr> expr_else) :
        expr_cond(std::move(expr_cond)), expr_then(std::move(expr_then)), expr_else(std::move(expr_else)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto cond_oid = expr_cond->eval(binding_id);

        ObjectId cond_bool = Conversions::to_boolean(cond_oid);

        switch (cond_bool.id) {
        case ObjectId::BOOL_TRUE:
            return expr_then->eval(binding_id);
        case ObjectId::BOOL_FALSE:
            return expr_else->eval(binding_id);
        default:
            return ObjectId::get_null();
        }
    }
};
