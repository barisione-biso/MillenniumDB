#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include <memory>
#include <vector>
#ifdef DEBUG
#include <string>
#endif

#include "base/graph/value/value.h"

class  Value {
public:
    virtual std::unique_ptr<std::vector<char>> get_bytes() = 0;
    #ifdef DEBUG
    virtual std::string to_string() = 0;
    #endif
};

#endif //BASE__VALUE_H_
