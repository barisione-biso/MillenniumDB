#ifndef BASE__OP_CONNECTION_H_
#define BASE__OP_CONNECTION_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpConnection : public Op {
public:
    const std::string from;
    const std::string to;
    const std::string edge;

    OpConnection(std::string from, std::string to, std::string edge) :
        from (std::move(from)),
        to   (std::move(to)),
        edge (std::move(edge)) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpConnection& other) const {
        return from < other.from && to < other.to && edge < other.edge;
    }
};

#endif // BASE__OP_CONNECTION_H_
