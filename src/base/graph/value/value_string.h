#ifndef BASE__VALUE_STRING_H_
#define BASE__VALUE_STRING_H_

#include "base/graph/value/value.h"

class ValueString : public Value {
private:
    std::string value;

public:
    ValueString(std::string value)
        : value(value) { }
    ~ValueString() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        int string_len = value.length();
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(string_len);
	    std::copy(value.begin(), value.end(), (*res).begin());
        return res;
    }

    std::string to_string() const {
        return value;
    }

    ValueType type() const {
        return ValueType::String;
    }

    bool operator==(const Value& rhs) const {
        if (rhs.type() == ValueType::String) {
            const auto& casted_rhs = dynamic_cast<const ValueString&>(rhs);
            return this->value == casted_rhs.value;
        }
        return false;
    }
};

#endif //BASE__VALUE_STRING_H_
