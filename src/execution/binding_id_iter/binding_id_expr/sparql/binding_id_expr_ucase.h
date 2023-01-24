#pragma once

#include <memory>
#include <locale>
#include <codecvt>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprUCase : public BindingIdExpr {
private:
    ObjectId pack_ucase(const std::string& str) const {
        std::locale locale("en_US.UTF-8"); // TODO: local computer may not have this locale
        std::wstring_convert<std::codecvt_utf8<wchar_t>> str_conv;
        auto wstr = str_conv.from_bytes(str);
        std::ctype<wchar_t> const &ctype_facet = std::use_facet<std::ctype<wchar_t> >(locale);
        for (auto& c : wstr) {
            c = ctype_facet.toupper(c);
        }
        auto ucase = str_conv.to_bytes(wstr);
        return Conversions::pack_string(ucase);
    }
public:
    std::unique_ptr<BindingIdExpr> expr;

    BindingIdExprUCase(std::unique_ptr<BindingIdExpr> expr) :
        expr (std::move(expr)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        switch(expr_oid.get_type()) {
        case ObjectId::MASK_STRING_INLINED: {
            std::string str = Conversions::unpack_string_inlined(expr_oid);
            return pack_ucase(str);
        }
        case ObjectId::MASK_STRING_EXTERN: {
            std::string str = Conversions::unpack_string_extern(expr_oid);
            return pack_ucase(str);
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
