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

    friend std::ostream& operator<<(std::ostream& os, const LiteralDatatypeTmp& graph_obj) {
        return os << '"' 
                  << (*graph_obj.ld).str
                  << '"'
                  << "^^"
                  << (*graph_obj.ld).datatype;
    }
};
