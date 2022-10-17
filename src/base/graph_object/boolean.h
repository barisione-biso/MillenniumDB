#pragma once

#include <cstring>
#include <cstdint>
#include <sstream>

class Boolean {
public:
    static constexpr uint64_t INVALID_ID = UINT64_MAX;

    static uint64_t get_boolean_id(const char* str) {
        if (strcmp(str, "true") == 0 || strcmp(str, "1") == 0) {
            return 1;
        }
        else if (strcmp(str, "false") == 0 || strcmp(str, "0") == 0) {
            return 0;
        }
        else {
            return INVALID_ID;
        }
    }

    bool id;

    Boolean(bool _id) : id(_id) { }

    inline bool operator==(const Boolean& rhs) const noexcept {
        return this->id == rhs.id;
    }

    inline bool operator!=(const Boolean& rhs) const noexcept {
        return this->id != rhs.id;
    }

    std::string to_string() const {
        std::stringstream ss;
        ss << '"';
        ss << (this->id ? "true" : "false");
        ss << '"'
           << "^^<http://www.w3.org/2001/XMLSchema#boolean>";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Boolean& b) {
        os << b.to_string();
        return os;
    }

};