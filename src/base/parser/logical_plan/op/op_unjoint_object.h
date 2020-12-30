#ifndef BASE__OP_UNJOINT_OBJECT_H_
#define BASE__OP_UNJOINT_OBJECT_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpUnjointObject : public Op {
public:
    const std::string obj_name;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpUnjointObject(" << obj_name << ")\n";
        return os;
    };

    OpUnjointObject(std::string obj_name) :
        obj_name (std::move(obj_name )) { }

    ~OpUnjointObject() = default;


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    bool operator<(const OpUnjointObject& other) const {
        return obj_name < other.obj_name;
    }
};

#endif // BASE__OP_UNJOINT_OBJECT_H_
