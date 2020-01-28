#ifndef BASE__OP_PROPERTY_H_
#define BASE__OP_PROPERTY_H_

#include "base/parser/logical_plan/op/op.h"
// #include "base/graph/value/variant_value.h"

#include <string>

class OpProperty : public Op {
public:
    const ElementType type;
    const std::string var;
    const std::string key;
    const ast::Value value;

    OpProperty(ElementType type, std::string var, std::string key, ast::Value value)
        : type(type), var(std::move(var)), key(std::move(key)), value(std::move(value)) { }

    void visit(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif //BASE__OP_PROPERTY_H_
