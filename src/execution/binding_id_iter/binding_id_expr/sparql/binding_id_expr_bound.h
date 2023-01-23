#pragma once

#include <memory>

#include "base/ids/var_id.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprBound : public BindingIdExpr {
public:
    VarId var_id;

    BindingIdExprBound(VarId var_id) :
        var_id (var_id) { }

    ObjectId eval(const BindingId& binding_id) const override {
        ObjectId oid = binding_id[var_id];
        if (oid.is_null())
            return ObjectId(ObjectId::BOOL_FALSE);
        else
            return ObjectId(ObjectId::BOOL_TRUE);
    }
};
