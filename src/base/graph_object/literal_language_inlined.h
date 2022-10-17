#pragma once

#include <cstring>
#include <ostream>
#include <stdint.h>
#include <string>

class LiteralLanguageInlined {
public:
    char     id[6];
    uint16_t language_id;

    LiteralLanguageInlined(const char* _id, uint8_t _language_id) :
        id { _id[0], _id[1], _id[2], _id[3], _id[4], '\0' }, language_id(_language_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const LiteralLanguageInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0 && this->language_id == rhs.language_id;
    }

    inline bool operator!=(const LiteralLanguageInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0 || this->language_id != rhs.language_id;
    }

    // inline bool operator<=(const LiteralLanguageInlined& rhs) const noexcept { }

    // inline bool operator>=(const LiteralLanguageInlined& rhs) const noexcept { }

    // inline bool operator<(const LiteralLanguageInlined& rhs) const noexcept { }

    // inline bool operator>(const LiteralLanguageInlined& rhs) const noexcept { }

    friend std::ostream& operator<<(std::ostream& os, const LiteralLanguageInlined& graph_obj) {
        return os << '"' 
                  << graph_obj.id
                  << '"'
                  << '@'
                  << StringManager::instance->get_language(graph_obj.language_id);
    }
};
