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
        if (edge < other.edge) {
            return true;
        } else if (type < other.type) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_CONNECTION_TYPE_H_
