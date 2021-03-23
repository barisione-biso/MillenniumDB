#ifndef BASE__OP_PATH_EPSILON_H_
#define BASE__OP_PATH_EPSILON_H_

#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op_path.h"

class OpPathEpsilon : public OpPath {
public:

    OpPathEpsilon() { }
    OpPathEpsilon(const OpPathEpsilon& _) { }

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
        os << "OpPathEpsilon()\n";

        return os;
    };

    bool nullable() const {
        return true;
    }

    std::unique_ptr<OpPath> duplicate() override {
        return std::make_unique<OpPathEpsilon>(*this);
    }
};

#endif // BASE__OP_PATH_EPSILON_H_
