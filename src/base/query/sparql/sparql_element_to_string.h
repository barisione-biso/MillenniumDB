#pragma once

#include <memory>
#include <string>

#include "base/graph_object/boolean.h"
#include "base/graph_object/datetime.h"
#include "base/graph_object/decimal.h"
#include "base/query/sparql/iri.h"
#include "base/query/sparql/literal.h"
#include "base/query/sparql/literal_datatype.h"
#include "base/query/sparql/literal_language.h"
#include "base/query/sparql/path.h"
#include "base/query/var.h"

struct SparqlElementToString {
    std::string operator()(const Var& v) {
        return "?" + v.name;
    }

    std::string operator()(const Iri& i) {
        return "<" + i.name + ">";
    }

    std::string operator()(const Literal& l) {
        return '"' + l.str + '"';
    }

    std::string operator()(const LiteralDatatype& ld) {
        return '"' + ld.str + '"' + "^^" + '<' + ld.datatype + '>';
    }

    std::string operator()(const LiteralLanguage& ll) {
        return '"' + ll.str + '"' + '@' + ll.language;
    }

    std::string operator()(const DateTime& dt) {
        return dt.to_string();
    }

    std::string operator()(const Decimal& d) {
        return d.to_string();
    }

    std::string operator()(const Boolean& b) {
        return b.to_string();
    }

    std::string operator()(const std::unique_ptr<SPARQL::IPath>& p) {
        return p.get()->to_string();
    }

    std::string operator()(int64_t i) {
        return std::to_string(i);
    }

    std::string operator()(float f) {
        return std::to_string(f);
    }
};
