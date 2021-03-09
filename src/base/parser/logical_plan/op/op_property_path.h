#ifndef BASE__OP_PROPERTY_PATH_H_
#define BASE__OP_PROPERTY_PATH_H_

#include <string>
#include <memory>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_path.h"

class OpPropertyPath : public Op {
public:
    const std::string from;
    const std::string to;
    std::unique_ptr<OpPath> path;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpPropertyPath( (" << from << ")=[" << *path << "]=>(" << to <<") )\n";
        return os;
    };

    OpPropertyPath(std::string from, std::string to, std::unique_ptr<OpPath> path) :
        from (std::move(from)),
        to   (std::move(to)),
        path (std::move(path))
        { }


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }


    bool operator<(const OpPropertyPath& other) const {
        if (from < other.from) {
            return true;
        } else if (to < other.to) {
            return true;
        } else if (*path < *other.path) {
            return true;
        }
        return false;
    }


    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (from[0] == '?') {
            res.insert(from);
        }
        if (to[0] == '?') {
            res.insert(to);
        }
        return res;
    }
};

#endif // BASE__OP_PROPERTY_PATH_H_
