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
    const Var edge;
    const std::vector<std::string> types; // TODO: use NodeId?

    OpConnection(NodeId _from, NodeId _to, Var _edge, std::vector<std::string> _types) :
        from  (_from),
        to    (_to),
        edge  (std::move(_edge)),
        types (std::move(_types)) { }

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

    void get_vars(std::set<Var>& set) const override {
        if (from.is_var()) {
            set.insert(from.to_var());
        }
        if (to.is_var()) {
            set.insert(to.to_var());
        }
        for (auto& type : types) {
            if (type[0] == '?') {
                set.emplace(type);
            }
        }
        set.insert(edge);
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpConnection( (" << from << ")-[" << edge << "]->(" << to <<") )\n";
        return os;
    };
};

#endif // BASE__OP_CONNECTION_H_
