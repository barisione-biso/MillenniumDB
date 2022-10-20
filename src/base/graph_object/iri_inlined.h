#pragma once

#include <cstring>
#include <ostream>
#include <string>
#include <stdint.h>

class IriInlined {
public:
    char    id[7];
    uint8_t prefix_id;

    IriInlined(const char* _id, uint8_t _prefix_id) :
        id { _id[0], _id[1], _id[2], _id[3], _id[4], _id[5], '\0' }, prefix_id(_prefix_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const IriInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0 && this->prefix_id == rhs.prefix_id;
    }

    inline bool operator!=(const IriInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0 || this->prefix_id != rhs.prefix_id;
    }

    // inline bool operator<=(const IriInlined& rhs) const noexcept { }

    // inline bool operator>=(const IriInlined& rhs) const noexcept { }

    // inline bool operator<(const IriInlined& rhs) const noexcept { }

    // inline bool operator>(const IriInlined& rhs) const noexcept { }

};
