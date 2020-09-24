#ifndef BASE__OP_PROPERTY_H_
#define BASE__OP_PROPERTY_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpProperty : public Op {
public:
    const std::string obj_name;
    const std::string key;
    const common::ast::Value value;

    OpProperty(std::string obj_name, std::string key, common::ast::Value value) :
        obj_name (std::move(obj_name) ),
        key      (std::move(key)      ),
        value    (std::move(value)    ) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    // Only comparing obj_name and key
    bool operator<(const OpProperty& other) const {
        if (obj_name < other.obj_name) {
            return true;
        } else if (key < other.key) {
            return true;
        }
        // not checking value
        return false;
    }
};

#endif // BASE__OP_PROPERTY_H_
