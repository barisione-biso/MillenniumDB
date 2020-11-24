#ifndef BASE__OP_TRANSITIVE_CLOSURE_H_
#define BASE__OP_TRANSITIVE_CLOSURE_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpTransitiveClosure : public Op {
public:
    const std::string from;
    const std::string to;
    const std::string type;

    OpTransitiveClosure(std::string from, std::string to, std::string type) :
        from (std::move(from)),
        to   (std::move(to)),
        type (std::move(type)) { }

    ~OpTransitiveClosure() = default;

    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    bool operator<(const OpTransitiveClosure& other) const {
        if (from < other.from) {
            return true;
        } else if (to < other.to) {
            return true;
        } else if (type < other.type) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_TRANSITIVE_CLOSURE_H_
