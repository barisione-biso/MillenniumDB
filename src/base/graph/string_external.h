#ifndef BASE__STRING_EXTERNAL_H_
#define BASE__STRING_EXTERNAL_H_

#include <cstring>

class StringExternal {
public:
    char* id;

    StringExternal() = delete;
    StringExternal(const char* id) :
        id (const_cast<char*>(id)) { }

    ~StringExternal() = default;

    inline void operator=(const StringExternal& other) {
        id[0] = other.id[0];
        id[1] = other.id[1];
        id[2] = other.id[2];
        id[3] = other.id[3];
        id[4] = other.id[4];
        id[5] = other.id[5];
        id[6] = other.id[6];
        id[7] = other.id[7];
    }

    inline bool operator==(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0;
    }

    inline bool operator!=(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0;
    }

    inline bool operator<=(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) <= 0;
    }

    inline bool operator>=(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) >= 0;
    }

    inline bool operator<(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) < 0;
    }

    inline bool operator>(const StringExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) > 0;
    }
};

#endif // BASE__STRING_EXTERNAL_H_
