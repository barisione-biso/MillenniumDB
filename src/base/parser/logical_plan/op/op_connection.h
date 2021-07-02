#ifndef BASE__OP_CONNECTION_H_
#define BASE__OP_CONNECTION_H_

#include <string>
#include <vector>

#include "base/ids/node_id.h"
#include "base/parser/logical_plan/op/op.h"

class OpConnection : public Op {
public:
    const NodeId from;
    const NodeId to;
    const std::string edge;
    const std::vector<std::string> types;

    OpConnection(NodeId from, NodeId to, std::string edge, std::vector<std::string> types) :
        from  (from),
        to    (to),
        edge  (std::move(edge)),
        types (std::move(types)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpConnection& other) const {
        // TODO: consider types?
        if (from < other.from) {
            return true;
        } else if (from > other.from) {
            return false;
        } else if (to < other.to) {
            return true;
        } else if (to > other.to) {
            return false;
        } else {
            return edge < other.edge;
        }
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        // TODO:
        // if (from[0] == '?') {
        //     res.insert(from);
        // }
        // if (to[0] == '?') {
        //     res.insert(to);
        // }
        if (edge[0] == '?') {
            res.insert(edge);
        }
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpConnection( (" << from << ")-[" << edge << "]->(" << to <<") )\n";
        return os;
    };
};

#endif // BASE__OP_CONNECTION_H_
