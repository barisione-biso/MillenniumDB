#pragma once

#include "base/query/sparql/literal_datatype.h"

class LiteralDatatypeTmp {
public:
    const LiteralDatatype* ld;

    LiteralDatatypeTmp(const LiteralDatatype& ld) :
        ld (&ld) { }

    inline bool operator==(const LiteralDatatypeTmp& rhs) const noexcept {
        return (*this->ld).str == (*rhs.ld).str &&
               (*this->ld).datatype == (*rhs.ld).datatype;
    }
};
