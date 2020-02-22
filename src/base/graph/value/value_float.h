#ifndef BASE__VALUE_FLOAT_H_
#define BASE__VALUE_FLOAT_H_

#include "base/graph/value/value.h"
#include "base/graph/value/value_int.h"

#include <cstring>

class ValueFloat: public Value {
public:
    const float value;

    ValueFloat(float value)
        : value(value) { }
    ~ValueFloat() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const override {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }

    std::string to_string() const override {
        return std::to_string(value);
    }

    ObjectType type() const override {
        return ObjectType::value_float;
    }

    bool operator==(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::value_float) {
            const auto& casted_rhs = static_cast<const ValueFloat&>(rhs);
            return this->value == casted_rhs.value;
        }
        // else if (rhs.type() == ObjectType::value_int) {
        //     const auto& casted_rhs = static_cast<const ValueInt&>(rhs);
        //     return this->value == casted_rhs.value;
        // }
        else return false;
    }
};


#endif // BASE__VALUE_FLOAT_H_
