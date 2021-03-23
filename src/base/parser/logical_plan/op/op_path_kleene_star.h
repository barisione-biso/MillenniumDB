#ifndef BASE__OP_PATH_KLEENE_STAR_H_
#define BASE__OP_PATH_KLEENE_STAR_H_

#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op_path.h"

class OpPathKleeneStar : public OpPath {
public:
    std::unique_ptr<OpPath> path;

    OpPathKleeneStar(std::unique_ptr<OpPath> path) :
        path (std::move(path))
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
        os << "OpPathKleeneStar()\n";
        path->print_to_ostream(os, indent +2);

        return os;
    };

    bool nullable() const {
        return true;
    }
};

#endif // BASE__OP_PATH_KLEENE_STAR_H_
