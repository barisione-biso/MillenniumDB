#ifndef BASE__IDENTIFIABLE_EXTERNAL_H_
#define BASE__IDENTIFIABLE_EXTERNAL_H_

#include <cstring>

class IdentifiableExternal {
public:
    const char* id;

    IdentifiableExternal(const char* id) :
        id (id) { }
    ~IdentifiableExternal() = default;

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
