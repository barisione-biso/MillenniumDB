#pragma once

#include "base/string_manager.h"

class LiteralLanguageExternal {
public:
    uint64_t external_id;

    LiteralLanguageExternal(uint64_t external_id) : external_id(external_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const LiteralLanguageExternal& rhs) const noexcept {
        return this->external_id == rhs.external_id;
    }

    inline bool operator!=(const LiteralLanguageExternal& rhs) const noexcept {
        return this->external_id != rhs.external_id;
    }

    // inline bool operator<=(const LiteralLanguageExternal& rhs) const noexcept { }

    // inline bool operator>=(const LiteralLanguageExternal& rhs) const noexcept { }

    // inline bool operator<(const LiteralLanguageExternal& rhs) const noexcept { }

    // inline bool operator>(const LiteralLanguageExternal& rhs) const noexcept { }

    friend std::ostream& operator<<(std::ostream& os, const LiteralLanguageExternal& graph_obj) {
        int      shift_size  = 5 * 8;
        uint16_t language_id = (graph_obj.external_id & 0x00FF'FF00'0000'0000UL) >> shift_size;
        uint64_t str_id      = graph_obj.external_id & 0x0000'00FF'FFFF'FFFFUL;
        auto     str         = StringManager::instance->get_string(str_id);

        return os << '"'
                  << str
                  << '"'
                  << "@"
                  << StringManager::instance->get_language(language_id);
    }
};
