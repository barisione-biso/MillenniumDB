#ifndef BASE__VALUE_STRING_H_
#define BASE__VALUE_STRING_H_

#include "base/graph/value/value.h"

class ValueString : public Value {
public:
    ValueString(std::string value)
        : value(value)
    {
    }
    ~ValueString() = default;

    std::unique_ptr<std::vector<char>> get_bytes() { // TODO: shared pointer? pass value?
        int string_len = value.length();
        std::unique_ptr<std::vector<char>> res = std::make_unique<std::vector<char>>(string_len);
	    std::copy(value.begin(), value.end(), (*res).begin());
        return res;
    }
    std::string to_string() {
        return value;
    }
private:
    std::string value;
};

#endif //BASE__VALUE_STRING_H_
