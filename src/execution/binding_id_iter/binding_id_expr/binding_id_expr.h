#pragma once

#include "base/binding/binding_id.h"
#include "base/ids/object_id.h"

// Abstract class
class BindingIdExpr {
public:
    virtual ~BindingIdExpr() = default;

    virtual ObjectId eval(const BindingId&) const = 0;
};
