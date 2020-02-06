#ifndef BASE__VALUE_BOOL_H
#define BASE__VALUE_BOOL_H

#include "base/graph/value/value.h"

class ValueBool: public Value {
private:
    bool value;

public:
    ValueBool(bool value)
        : value(value) { }
    ~ValueBool() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        return std::make_unique<std::vector<char>>(1, (char)value);
    }

    std::string to_string() const {
        return std::to_string(value);
    }

    ValueType type() const {
        return ValueType::Bool;
    }

    bool operator==(const Value& rhs) const {
        if (rhs.type() == ValueType::Bool) {
            const auto& casted_rhs = dynamic_cast<const ValueBool&>(rhs);
            return this->value == casted_rhs.value;
        }
        return false;
    }
};


#endif // BASE__VALUE_BOOL_H