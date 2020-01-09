#ifndef BASE__VALUE_INT_H
#define BASE__VALUE_INT_H

#include "base/graph/value/value.h"

#include <cstring>

class ValueInt: public Value {
public:
    ValueInt(int32_t value)
        : value(value)
    {
    }
    ~ValueInt() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(sizeof(value));
	    std::memcpy((*res).data(), &value, sizeof(value));
        return res;
    }
    std::string to_string() {
        return std::to_string(value);
    }


private:
    int32_t value;
};


#endif // BASE__VALUE_INT_H