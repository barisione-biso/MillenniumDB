#ifndef BASE__VALUE_FLOAT_H
#define BASE__VALUE_FLOAT_H

#include "base/graph/value/value.h"

#include <cstring>

class ValueFloat: public Value {

private:
    float value;

public:
    ValueFloat(float value)
        : value(value) { }
    ~ValueFloat() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }

    std::string to_string() const {
        return std::to_string(value);
    }

    ValueType type() const {
        return ValueType::Float;
    }

    bool operator==(const Value& rhs) const {
        if (rhs.type() == ValueType::Float) {
            const auto& casted_rhs = dynamic_cast<const ValueFloat&>(rhs);
            return this->value == casted_rhs.value;
        }
        return false;
    }
};


#endif // BASE__VALUE_FLOAT_H