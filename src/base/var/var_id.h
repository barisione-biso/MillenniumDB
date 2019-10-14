#ifndef BASE__VAR_ID_H_
#define BASE__VAR_ID_H_

class VarId {
public:
    VarId(int var_id);
    ~VarId() = default;

    int const var_id;
    bool is_null();

    /*VarId& operator=(const VarId& copy) {
        var_id = copy.var_id;
        return *this;
    }*/

    bool operator <(const VarId& rhs) const {
        return var_id < rhs.var_id;
    }

    bool operator ==(const VarId& rhs) const {
        return var_id == rhs.var_id;
    }

    bool operator !=(const VarId& rhs) const {
        return var_id != rhs.var_id;
    }

    static VarId null_var;
};

#endif //BASE__VAR_ID_H_
