#pragma once

#include <cassert>
#include <memory>
#include <variant>

#include "base/graph_object/boolean.h"
#include "base/graph_object/datetime.h"
#include "base/graph_object/decimal.h"
#include "base/query/sparql/iri.h"
#include "base/query/sparql/literal.h"
#include "base/query/sparql/literal_datatype.h"
#include "base/query/sparql/literal_language.h"
#include "base/query/sparql/path.h"
#include "base/query/var.h"

class SparqlElement {
public:
    // TODO: replace literal with string?
    std::variant<Var, Iri, Literal, LiteralDatatype, LiteralLanguage, DateTime, Decimal, Boolean, std::unique_ptr<SPARQL::IPath>> value;

    SparqlElement() : value(Boolean(false)) { }

    SparqlElement(const Var& var) : value(var) { }

    SparqlElement(const Iri& iri) : value(iri) { }

    SparqlElement(const Literal& literal) : value(literal) { }

    SparqlElement(const LiteralDatatype& literal_datatype) : value(literal_datatype) { }

    SparqlElement(const LiteralLanguage& literal_language) : value(literal_language) { }

    SparqlElement(DateTime datetime) : value(datetime) { }

    SparqlElement(Decimal decimal) : value(decimal) { }

    SparqlElement(Boolean boolean) : value(boolean) { }

    SparqlElement(std::unique_ptr<SPARQL::IPath> path) : value(std::move(path)) { }

    // SparqlElement(const int64_t i) : value(i) { }

    // SparqlElement(const float f) : value(f) { }

    SparqlElement duplicate() const;

    inline bool is_var() const {
        return std::holds_alternative<Var>(value);
    }

    inline bool is_path() const {
        return std::holds_alternative<std::unique_ptr<SPARQL::IPath>>(value);
    }

    inline bool is_bnode() const {
        // Blank nodes are treated as variables.
        // To identify them, we check if the first character is '_'.
        return std::holds_alternative<Var>(value) && to_var().name[0] == '_';
    }

    inline Var to_var() const {
        assert(is_var());
        return std::get<Var>(value);
    }

    friend std::ostream& operator<<(std::ostream& os, const SparqlElement& node_id) {
        // TODO: implement
        return os;
    }
};