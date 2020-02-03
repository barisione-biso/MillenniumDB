#ifndef BASE__VALUE_VAR_H
#define BASE__VALUE_VAR_H

#include "base/graph/value/value.h"

#include <cstring>

class ValueVar: public Value {
private:
    VarId var_id;
public:
    ValueVar(VarId var_id)
        : var_id(var_id)
    {
    }
    ~ValueVar() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        return nullptr;
    }

    std::string to_string() {
        return "VarId(" + std::to_string(var_id) + ")";
    }

    ValueType type() {
        return ValueType::Var;
    }
};


#endif // BASE__VALUE_INT_H