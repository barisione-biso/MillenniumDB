#ifndef BASE__VALUE_INT_H_
#define BASE__VALUE_INT_H_

#include "base/graph/value/value.h"
#include "base/graph/value/value_float.h"

#include <cstring>

class ValueInt: public Value {
public:
    const int32_t value;

    ValueInt(int32_t value)
        : value(value) { }
    ~ValueInt() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const override {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }

    std::string to_string() const override {
        return std::to_string(value);
    }

    ObjectType type() const override {
        return ObjectType::value_int;
    }

    bool operator==(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::value_int) {
            const auto& casted_rhs = static_cast<const ValueInt&>(rhs);
            return this->value == casted_rhs.value;
        }
        // else if (rhs.type() == ObjectType::value_float) {
        //     const auto& casted_rhs = static_cast<const ValueFloat&>(rhs);
        //     return this->value == casted_rhs.value;
        // }
        else return false;
    }
};

#endif // BASE__VALUE_INT_H_
