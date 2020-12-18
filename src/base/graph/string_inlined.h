#ifndef BASE__STRING_INLINED_H_
#define BASE__STRING_INLINED_H_

#include <cassert>
#include <cstring>

class StringInlined {
public:
    char id[8];

    StringInlined() = delete;
    StringInlined(const char* _id) :
        id {_id[0], _id[1], _id[2], _id[3], _id[4], _id[5], _id[6], '\0'}
    {
        assert(_id[7] == '\0');
    }

    ~StringInlined() = default;

    inline void operator=(const StringInlined& other) {
        id[0] = other.id[0];
        id[1] = other.id[1];
        id[2] = other.id[2];
        id[3] = other.id[3];
        id[4] = other.id[4];
        id[5] = other.id[5];
        id[6] = other.id[6];
        id[7] = other.id[7];
    }

    inline bool operator==(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0;
    }

    inline bool operator!=(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0;
    }

    inline bool operator<=(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) <= 0;
    }

    inline bool operator>=(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) >= 0;
    }

    inline bool operator<(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) < 0;
    }

    inline bool operator>(const StringInlined& rhs) const noexcept {
        return strcmp(this->id, rhs.id) > 0;
    }
};

#endif // BASE__STRING_INLINED_H_
