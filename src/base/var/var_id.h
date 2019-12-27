#ifndef BASE__VAR_ID_H_
#define BASE__VAR_ID_H_

#include <cstdint>

class VarId {
public:
    // static VarId null;
    // static VarId term;

    int_fast32_t id;
    VarId(int_fast32_t id);
    ~VarId() = default;

    // bool is_null();

    operator int_fast32_t() const {
        return id;
    }

    /*VarId& operator=(const VarId& copy) {
        var_id = copy.var_id;
        return *this;
    }*/

    bool operator <(const VarId& rhs) const {
        return id < rhs.id;
    }

    bool operator ==(const VarId& rhs) const {
        return id == rhs.id;
    }

    bool operator !=(const VarId& rhs) const {
        return id != rhs.id;
    }

};

#endif //BASE__VAR_ID_H_
