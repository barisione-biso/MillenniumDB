#ifndef BASE__IDENTIFIABLE_EXTERNAL_H_
#define BASE__IDENTIFIABLE_EXTERNAL_H_

#include <cstring>

class IdentifiableExternal {
public:
    char* id;

    IdentifiableExternal(const char* id) :
        id (const_cast<char*>(id)) { }
    ~IdentifiableExternal() = default;

    inline void operator=(const IdentifiableExternal& other) {
        id = other.id;
    }

    inline bool operator==(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) == 0;
    }

    inline bool operator!=(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) != 0;
    }

    inline bool operator<=(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) <= 0;
    }

    inline bool operator>=(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) >= 0;
    }

    inline bool operator<(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) < 0;
    }

    inline bool operator>(const IdentifiableExternal& rhs) const noexcept {
        return strcmp(this->id, rhs.id) > 0;
    }
};

#endif // BASE__IDENTIFIABLE_EXTERNAL_H_
