#pragma once

#include <memory>
#include <string>

#include "base/query/sparql/sparql_element.h"

struct SparqlElementDuplicate {
    SparqlElement operator()(const Var& v) {
        return SparqlElement(v);
    }

    SparqlElement operator()(const Iri& i) {
        return SparqlElement(i);
    }

    SparqlElement operator()(const Literal& l) {
        return SparqlElement(l);
    }

    SparqlElement operator()(const LiteralDatatype& ld) {
        return SparqlElement(ld);
    }

    SparqlElement operator()(const LiteralLanguage& ll) {
        return SparqlElement(ll);
    }

    SparqlElement operator()(const DateTime& dt) {
        return SparqlElement(dt);
    }

    SparqlElement operator()(const Decimal& d) {
        return SparqlElement(d);
    }

    SparqlElement operator()(bool b) {
        return SparqlElement(b);
    }

    SparqlElement operator()(const std::unique_ptr<IPath>& p) {
        return SparqlElement(p->duplicate());
    }

    SparqlElement operator()(int64_t i) {
        return SparqlElement(i);
    }

    SparqlElement operator()(float f) {
        return SparqlElement(f);
    }
};
