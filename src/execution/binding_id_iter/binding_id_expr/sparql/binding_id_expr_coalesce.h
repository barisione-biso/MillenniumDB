#pragma once

#include <memory>

#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprCoalesce : public BindingIdExpr {
public:
    std::vector<std::unique_ptr<BindingIdExpr>> exprs;

    BindingIdExprCoalesce(std::vector<std::unique_ptr<BindingIdExpr>> exprs) :
        exprs(std::move(exprs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        for (auto& expr : exprs) {
            auto expr_oid = expr->eval(binding_id);
            if (!expr_oid.is_null())
                return expr_oid;
        }
        return ObjectId::get_null();
    }
};
