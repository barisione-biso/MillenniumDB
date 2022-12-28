#pragma once

#include <memory>

#include "base/ids/object_id.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprTerm : public BindingIdExpr {
public:
    ObjectId object_id;

    BindingIdExprTerm(ObjectId object_id) :
        object_id(object_id) { }

    ObjectId eval(const BindingId&) const override {
        return object_id;
    }
};
