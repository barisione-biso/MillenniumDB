#pragma once

#include <memory>

#include "base/ids/object_id_conversions.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class BindingIdExprIRI : public BindingIdExpr {
public:
    std::unique_ptr<BindingIdExpr> expr;
    std::string base_iri;

    BindingIdExprIRI(std::unique_ptr<BindingIdExpr> expr, std::string base_iri) :
        expr (std::move(expr)), base_iri (base_iri) { }

    ObjectId eval(const BindingId& binding_id) const override {
        auto expr_oid = expr->eval(binding_id);

        uint64_t type = expr_oid.get_type();

        // IRIs remain unchanged
        if (type == ObjectId::MASK_IRI_INLINED || type == ObjectId::MASK_IRI_EXTERN) {
            return expr_oid;
        }
        // Strings are converted to IRIs
        else if (type == ObjectId::MASK_STRING_INLINED) {
            std::string str = Conversions::unpack_string_inlined(expr_oid);
            if (str.find(':') == std::string::npos) {
                // IRI is not absolute
                if (base_iri.empty())
                    return ObjectId::get_null();
                else
                    return Conversions::pack_iri(base_iri + str);
            }
            else {
                // IRI is absolute
                return Conversions::pack_iri(str);
            }
        }
        else if (type == ObjectId::MASK_STRING_EXTERN) {
            std::string str = Conversions::unpack_string_extern(expr_oid);
            // If IRI is not absolute and base_iri is empty, return null
            if (str.find(':') == std::string::npos) {
                // IRI is not absolute
                if (base_iri.empty()) 
                    return ObjectId::get_null();
                else 
                    return Conversions::pack_iri(base_iri + str);
            }
            else {
                // IRI is absolute
                return Conversions::pack_iri(str);
            }
        }
        // Other types return null
        else {
            return ObjectId::get_null();
        }
    }
};
