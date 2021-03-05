#ifndef BASE__OP_ZERO_OR_ONE_PATH_H_
#define BASE__OP_ZERO_OR_ONE_PATH_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpZeroOrOnePath : public Op {
public:
    const std::string from;
    const std::string to;
    const std::string type;

    OpZeroOrOnePath(std::string from, std::string to, std::string type) :
        from (std::move(from)),
        to   (std::move(to)),
        type (std::move(type)) { }

    ~OpZeroOrOnePath() = default;


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpZeroOrOnePath& other) const {
        if (from < other.from) {
            return true;
        } else if (to < other.to) {
            return true;
        } else if (type < other.type) {
            return true;
        }
        return false;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpZeroOrOnePath(" << from << "->" << to  << ":" << type << ")\n";
        return os;
    };

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (from[0] == '?') {
            res.insert(from);
        }
        if (to[0] == '?') {
            res.insert(to);
        }
        if (type[0] == '?') {
            res.insert(type);
        }
        return res;
    }
};

#endif // BASE__OP_ZERO_OR_ONE_PATH_H_
