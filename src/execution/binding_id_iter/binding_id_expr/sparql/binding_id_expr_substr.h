#pragma once

#include <memory>

#include <boost/locale.hpp>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprSubStr : public BindingIdExpr {
private:
    ObjectId pack_substr(const std::string& str, size_t start, size_t length) const {
        // Fix offset
        if (start != 0)
            start--;
        else
            length--;
        // Fast path for empty strings
        if (str.empty() || start > str.size() || length == 0)
            return ObjectId(ObjectId::STRING_EMPTY);
        // TODO: Inefficient?
        // Convert to wstring and get the substring
        // This considers the unicode multi-byte characters
        std::wstring wstr    = boost::locale::conv::to_utf<wchar_t>(str, "UTF-8");
        std::wstring subwstr = wstr.substr(start, length);
        std::string  substr  = boost::locale::conv::from_utf(subwstr, "UTF-8");
        return Conversions::pack_string(substr);
    }

public:
    std::unique_ptr<BindingIdExpr> expr_str;
    std::unique_ptr<BindingIdExpr> expr_start;
    // may be nullptr
    std::unique_ptr<BindingIdExpr> expr_length;

    BindingIdExprSubStr(std::unique_ptr<BindingIdExpr> expr_str,
                        std::unique_ptr<BindingIdExpr> expr_start) :
        expr_str   (std::move(expr_str)),
        expr_start (std::move(expr_start)) { }

    BindingIdExprSubStr(std::unique_ptr<BindingIdExpr> expr_str,
                        std::unique_ptr<BindingIdExpr> expr_start,
                        std::unique_ptr<BindingIdExpr> expr_length) :
        expr_str    (std::move(expr_str)),
        expr_start  (std::move(expr_start)),
        expr_length (std::move(expr_length)) { }

    ObjectId eval(const BindingId& binding_id) const override {
        size_t start  = 0;
        size_t length = std::wstring::npos;

        // This function could support any numeric type for start and length
        // but this implementation only supports positive integers by now
        // Sources:
        // 1. https://www.w3.org/TR/sparql11-query/#func-substr
        // 2. https://www.w3.org/TR/xpath-functions/#func-substring

        // Start must be a positive integer
        auto start_oid = expr_start->eval(binding_id);
        if (start_oid.get_type() != ObjectId::MASK_POSITIVE_INT)
            return ObjectId::get_null();
        else
            start = Conversions::unpack_positive_int(start_oid);

        // Length must be a positive integer or undefined
        auto length_oid = ObjectId::get_null();
        if (expr_length != nullptr) {
            length_oid = expr_length->eval(binding_id);
            if (length_oid.get_type() != ObjectId::MASK_POSITIVE_INT)
                return ObjectId::get_null();
            else
                length = Conversions::unpack_positive_int(length_oid);
        }

        auto str_oid = expr_str->eval(binding_id);
        switch (str_oid.get_type()) {
        case ObjectId::MASK_STRING_INLINED: {
            std::string str = Conversions::unpack_string_inlined(str_oid);
            return pack_substr(str, start, length);
        }
        case ObjectId::MASK_STRING_EXTERN: {
            std::string str = Conversions::unpack_string_extern(str_oid);
            return pack_substr(str, start, length);
        }
        // TODO: Use temporary strings
        case ObjectId::MASK_STRING_LANG_INLINED:
            throw std::runtime_error("Not implemented yet: SUBSTR/ObjectId::MASK_STRING_LANG_INLINED");
        case ObjectId::MASK_STRING_LANG_EXTERN:
            throw std::runtime_error("Not implemented yet: SUBSTR/ObjectId::MASK_STRING_LANG_EXTERN");
        default:
            return ObjectId::get_null();
        }
    }
};
