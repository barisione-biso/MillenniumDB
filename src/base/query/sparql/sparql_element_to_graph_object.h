#pragma once

#include "base/exceptions.h"
#include "base/graph_object/boolean.h"
#include "base/graph_object/datetime.h"
#include "base/graph_object/decimal.h"
#include "base/graph_object/graph_object.h"
#include "base/query/sparql/iri.h"
#include "base/query/sparql/literal.h"
#include "base/query/sparql/literal_datatype.h"
#include "base/query/sparql/literal_language.h"
#include "base/query/sparql/path.h"
#include "base/query/var.h"
#include "query_optimizer/rdf_model/rdf_model.h"

struct SparqlElementToGraphObject {
    GraphObject operator()(const Var& /*v*/) {
        throw LogicException("Var cannot be converted into graph_object");
    }

    GraphObject operator()(const Iri& iri) {
        auto& prefixes = rdf_model.catalog().prefixes;
        std::string str = iri.name;
        uint8_t prefix_id = 0;
        for (size_t i = 0; i < prefixes.size(); ++i) {
            if (str.compare(0, prefixes[i].size(), prefixes[i]) == 0) {
                str = str.substr(prefixes[i].size(), str.size() - prefixes[i].size());
                prefix_id = i;
                break;
            }
        }
        if (str.size() < 7) {
            return GraphObject::make_iri_inlined(str.c_str(), prefix_id);
        } else {
            return GraphObject::make_iri_tmp(iri.name);
        }
    }

    GraphObject operator()(const Literal& l) {
        return GraphObject::make_string(l.str);
    }

    GraphObject operator()(const LiteralDatatype& ld) {
        auto& datatypes = rdf_model.catalog().datatypes;
        if (ld.str.size() < 6) {
            for (size_t i = 0; i < datatypes.size(); ++i) {
                if (ld.datatype == datatypes[i]) {
                    return GraphObject::make_literal_datatype_inlined(ld.str.c_str(), i);
                }
            }
        }
        // Not inlined str or datatype not found
        return GraphObject::make_literal_datatype_tmp(ld);
    }

    GraphObject operator()(const LiteralLanguage& ll) {
        auto languages = rdf_model.catalog().languages;
        if (ll.str.size() < 6) {
            for (size_t i = 0; i < languages.size(); ++i) {
                if (ll.language == languages[i]) {
                    return GraphObject::make_literal_language_inlined(ll.str.c_str(), i);
                }
            }
        }
        // Not inlined str or language not found
        return GraphObject::make_literal_language_tmp(ll);
    }

    GraphObject operator()(const DateTime& dt) {
        return GraphObject::make_datetime(dt.id);
    }

    GraphObject operator()(const Decimal& d) {
        return GraphObject::make_decimal(d.id);
    }

    GraphObject operator()(const Boolean& b) {
        return GraphObject::make_boolean(b.id);
    }

    GraphObject operator()(const std::unique_ptr<SPARQL::IPath>& /*p*/) {
        // TODO: Implement this
        return GraphObject::make_string("PATH");
    }

    GraphObject operator()(int64_t i) {
        // TODO: Treat integers as integers instead of literal datatype
        return GraphObject::make_int(i);
    }

    GraphObject operator()(float f) {
        // TODO: Treat floats as floats instead of literal datatype
        return GraphObject::make_float(f);
    }
};
