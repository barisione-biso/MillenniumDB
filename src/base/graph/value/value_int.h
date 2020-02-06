#ifndef BASE__VALUE_INT_H_
#define BASE__VALUE_INT_H_

#include "base/graph/value/value.h"

#include <cstring>

class ValueInt: public Value {
private:
    int32_t value;

public:
    ValueInt(int32_t value)
        : value(value) { }
    ~ValueInt() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }

    std::string to_string() const {
        return std::to_string(value);
    }

    ValueType type() const {
        return ValueType::Int;
    }

    bool operator==(const Value& rhs) const {
        if (rhs.type() == ValueType::Int) {
            const auto& casted_rhs = dynamic_cast<const ValueInt&>(rhs);
            return this->value == casted_rhs.value;
        }
        return false;
    }
};

#endif // BASE__VALUE_INT_H_
