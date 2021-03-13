#ifndef BASE__OP_PATH_ALTERNATIVES_H_
#define BASE__OP_PATH_ALTERNATIVES_H_

#include <string>


#include "base/parser/logical_plan/op/op_path.h"

class OpPathAlternatives : public OpPath {
public:
    std::vector<std::unique_ptr<OpPath>> alternatives;

    OpPathAlternatives(std::vector<std::unique_ptr<OpPath>> alternatives) :
        alternatives (std::move(alternatives))
        { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpPath& other) const override {
        // TODO:
        // if (min < min) {
        //     return true;
        // } else if (max < max) {
        //     return true;
        // } else if (*op_path < *other.op_path) {
        //     return true;
        // }
        return false;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpAlternatives()\n";

        for (auto& alternative : alternatives) {
            alternative->print_to_ostream(os, indent + 2);
        }

        return os;
    };
};

#endif // BASE__OP_PATH_ALTERNATIVES_H_
