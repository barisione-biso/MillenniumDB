#ifndef BASE__VALUE_FLOAT_H
#define BASE__VALUE_FLOAT_H

#include "base/graph/value/value.h"

#include <cstring>

class ValueFloat: public Value {
public:
    ValueFloat(float value)
        : value(value)
    {
    }
    ~ValueFloat() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const { 
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }
    std::string to_string() {
        return std::to_string(value);
    }


private:
    float value;
};


#endif // BASE__VALUE_FLOAT_H