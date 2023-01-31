#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"
#include "third_party/hashes/sha384.h"

class BindingIdExprSHA384 : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprSHA384(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);
;
        switch (expr_oid.get_type()) {
        // TODO: Use temporary strings
        case ObjectId::MASK_STRING_INLINED: {
            SHA384 sha384;
            std::string str  = Conversions::unpack_string_inlined(expr_oid);
            std::string hash = sha384.hash(str);
            return Conversions::pack_string(hash);
        }
        case ObjectId::MASK_STRING_EXTERN: {
            SHA384 sha384;
            std::string str  = Conversions::unpack_string_extern(expr_oid);
            std::string hash = sha384.hash(str);
            return Conversions::pack_string(hash);
        }
        default:
            return ObjectId::get_null();
        }
    }
};
