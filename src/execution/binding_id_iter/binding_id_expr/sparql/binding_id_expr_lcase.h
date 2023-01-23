#pragma once

#include <memory>

#include <boost/locale.hpp>
#include <boost/locale/generator.hpp>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprLCase : public BindingIdExpr {
private:
    ObjectId pack_lcase(const std::string& str) const {
        std::locale loc   = boost::locale::generator()("en_US.UTF-8");
        std::string lcase = boost::locale::to_lower(str, loc);
        return Conversions::pack_string(lcase);
    }

public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprLCase(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }    

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch(expr_oid.get_type()) {
        case ObjectId::MASK_STRING_INLINED: {
            std::string str = Conversions::unpack_string_inlined(expr_oid);
            return pack_lcase(str);
        }
        case ObjectId::MASK_STRING_EXTERN: {
            std::string str = Conversions::unpack_string_extern(expr_oid);
            return pack_lcase(str);
        }
        // TODO: Use temporary strings
        case ObjectId::MASK_STRING_LANG_INLINED:
            throw std::runtime_error("Not implemented yet: LCASE/ObjectId::MASK_STRING_LANG_INLINED");
        case ObjectId::MASK_STRING_LANG_EXTERN: 
            throw std::runtime_error("Not implemented yet: LCASE/ObjectId::MASK_STRING_LANG_EXTERN");
        default:
            return ObjectId::get_null();
        }
    }
};
