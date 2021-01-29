#ifndef BASE__OP_PROPERTY_H_
#define BASE__OP_PROPERTY_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpProperty : public Op {
public:
    const std::string obj_name;
    const std::string key;
    const common::ast::Value value;

    std::ostream& print_to_ostream(std::ostream& os,  int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpProperty(" << key << ":" << obj_name << ")\n";
        return os;
    };

    OpProperty(std::string obj_name, std::string key, common::ast::Value value) :
        obj_name (std::move(obj_name) ),
        key      (std::move(key)      ),
        value    (std::move(value)    ) { }


    void accept_visitor(OpVisitor& visitor) override {
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

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (obj_name[0] == '?') {
            res.insert(obj_name);
        }
        // we assume key won't be a variable
        // we assume value won't be a variable
        return res;
    }
};

#endif // BASE__OP_PROPERTY_H_
