#pragma once

#include "base/string_manager.h"

class IriExternal {
public:
    uint64_t external_id;

    IriExternal(uint64_t external_id) : external_id(external_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const IriExternal& rhs) const noexcept {
        return this->external_id == rhs.external_id;
    }

    inline bool operator!=(const IriExternal& rhs) const noexcept {
        return this->external_id != rhs.external_id;
    }

    // inline bool operator<=(const IriExternal& rhs) const noexcept { }

    // inline bool operator>=(const IriExternal& rhs) const noexcept { }

    // inline bool operator<(const IriExternal& rhs) const noexcept { }

    // inline bool operator>(const IriExternal& rhs) const noexcept { }

    friend std::ostream& operator<<(std::ostream& os, const IriExternal& graph_obj) {
        int  shift_size = 6 * 8;
        uint8_t prefix_id = (graph_obj.external_id & 0x00FF'0000'0000'0000UL) >> shift_size;
        uint64_t iri_id = graph_obj.external_id & 0x0000'FFFF'FFFF'FFFFUL;

        return os << '<'
                  << StringManager::instance->get_prefix(prefix_id)
                  << StringManager::instance->get_string(iri_id)
                  << '>';
    }
};
