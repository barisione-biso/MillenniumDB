#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprOr : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> lhs;
    std::unique_ptr<BindingIdExpr> rhs;

    BindingIdExprOr(std::unique_ptr<BindingIdExpr> lhs, std::unique_ptr<BindingIdExpr> rhs) :
        lhs (std::move(lhs)),
        rhs (std::move(rhs)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto lhs_value = lhs->eval(binding_id);
        auto rhs_value = rhs->eval(binding_id);
        
        ObjectId lhs_bool = Conversions::to_boolean(lhs_value);
        ObjectId rhs_bool = Conversions::to_boolean(rhs_value);

        // Evaluation according to the SPARQL 1.1 standard
        // https://www.w3.org/TR/sparql11-query/#evaluation
        if (lhs_bool.id == ObjectId::BOOL_TRUE || rhs_bool.id == ObjectId::BOOL_TRUE) {
            return ObjectId(ObjectId::BOOL_TRUE);
        } else if (lhs_bool.id == ObjectId::BOOL_FALSE && rhs_bool.id == ObjectId::BOOL_FALSE) {
            return lhs_bool;
        } else {
            return ObjectId(ObjectId::NULL_OBJECT_ID);
        }
    }
};
