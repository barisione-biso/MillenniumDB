#ifndef BASE__OP_CONNECTION_TYPE_H_
#define BASE__OP_CONNECTION_TYPE_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpConnectionType : public Op {
public:
    const std::string edge;
    const std::string type;

    OpConnectionType(std::string edge, std::string type) :
        edge (std::move(edge)),
        type (std::move(type)) { }

    ~OpConnectionType() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpConnectionType& other) const {
        return edge < other.edge && type < other.type;
    }
};

#endif // BASE__OP_CONNECTION_TYPE_H_
