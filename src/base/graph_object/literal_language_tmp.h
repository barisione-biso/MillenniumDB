#pragma once

#include "base/query/sparql/literal_language.h"

class LiteralLanguageTmp {
public:
    const LiteralLanguage* ll;

    LiteralLanguageTmp(const LiteralLanguage& ll) :
        ll (&ll) { }

    inline bool operator==(const LiteralLanguageTmp& rhs) const noexcept {
        return (*this->ll).str == (*rhs.ll).str &&
               (*this->ll).language == (*rhs.ll).language;
    }
};
