#pragma once

#include <memory>

#include "base/ids/var_id.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprVar : public BindingIdExpr {
public:
    VarId var_id;

    BindingIdExprVar(VarId var_id) :
        var_id (var_id) { }

    ObjectId eval(const BindingId& binding_id) const override {
        return binding_id[var_id];
    }
};
