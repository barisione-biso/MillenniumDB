#ifndef BASE__OP_CONNECTION_H_
#define BASE__OP_CONNECTION_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpConnection : public Op {
public:
    const std::string from;
    const std::string to;
    const std::string edge;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpConnection(" << from << "-" << edge << "->" << to <<")\n";
        return os;
    };

    OpConnection(std::string from, std::string to, std::string edge) :
        from (std::move(from)),
        to   (std::move(to)),
        edge (std::move(edge)) { }


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }


    bool operator<(const OpConnection& other) const {
        // return from < other.from && to < other.to && edge < other.edge;
        if (from < other.from) {
            return true;
        } else if (to < other.to) {
            return true;
        } else if (edge < other.edge) {
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
        if (edge[0] == '?') {
            res.insert(edge);
        }
        return res;
    }
};

#endif // BASE__OP_CONNECTION_H_
