#ifndef BASE__VALUE_BOOL_H
#define BASE__VALUE_BOOL_H

#include "base/graph/value/value.h"

class ValueBool: public Value {
public:
    ValueBool(bool value)
        : value(value)
    {
    }
    ~ValueBool() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(1, (char)value);
        return res;
    }
    std::string to_string() {
        return std::to_string(value);
    }

    bool is_var(){ return false; }
    VarId get_var() { throw std::bad_cast(); }


private:
    bool value;
};


#endif // BASE__VALUE_BOOL_H