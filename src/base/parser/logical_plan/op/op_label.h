#ifndef BASE__OP_LABEL_H_
#define BASE__OP_LABEL_H_

#include "base/graph/element_type.h"
#include "base/parser/logical_plan/op/op.h"

#include <string>

class OpLabel : public Op {
public:
    const ElementType type;
    const std::string var;
    const std::string label;

    OpLabel(ElementType type, std::string var, std::string label)
        : type(type), var(std::move(var)), label(std::move(label)) { }

    ~OpLabel() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif //BASE__OP_LABEL_H_
