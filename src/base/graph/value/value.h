#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include "base/graph/graph_object.h"

#include <memory>
#include <string>
#include <vector>

class Value : public GraphObject {
public:
    virtual std::unique_ptr<std::vector<unsigned char>> get_bytes() const = 0;
    virtual std::string to_string() const = 0;
    virtual ObjectType type() const = 0;

    virtual bool operator==(const GraphObject& rhs) const = 0;
};

#endif //BASE__VALUE_H_
