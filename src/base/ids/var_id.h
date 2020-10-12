#ifndef BASE__VAR_ID_H_
#define BASE__VAR_ID_H_

#include <cstdint>

class VarId {
public:
    const uint_fast32_t id;

    explicit VarId(uint_fast32_t id)
        : id(id) { }

    ~VarId() = default;

    inline bool operator<(const VarId& rhs) const noexcept {
        return id < rhs.id;
    }

    inline bool operator==(const VarId& rhs) const noexcept {
        return id == rhs.id;
    }

    inline bool operator!=(const VarId& rhs) const noexcept {
        return id != rhs.id;
    }
};

#endif // BASE__VAR_ID_H_
