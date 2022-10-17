#pragma once

#include <cstring>
#include <ostream>
#include <stdint.h>
#include <string>
#include "base/string_manager.h"

class LiteralDatatypeInlined {
public:
    char     id[6];
    uint16_t datatype_id;

    LiteralDatatypeInlined(const char* _id, uint8_t _datatype_id) :
        id { _id[0], _id[1], _id[2], _id[3], _id[4], '\0' }, datatype_id(_datatype_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const LiteralDatatypeInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0 && this->datatype_id == rhs.datatype_id;
    }

    inline bool operator!=(const LiteralDatatypeInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0 || this->datatype_id != rhs.datatype_id;
    }

    // inline bool operator<=(const LiteralDatatypeInlined& rhs) const noexcept { }

    // inline bool operator>=(const LiteralDatatypeInlined& rhs) const noexcept { }

    // inline bool operator<(const LiteralDatatypeInlined& rhs) const noexcept { }

    // inline bool operator>(const LiteralDatatypeInlined& rhs) const noexcept { }

    friend std::ostream& operator<<(std::ostream& os, const LiteralDatatypeInlined& graph_obj) {
        return os << '"' 
                  << graph_obj.id
                  << '"' 
                  << "^^" 
                  << '<' 
                  << StringManager::instance->get_datatype(graph_obj.datatype_id)
                  << '>';
    }
};
