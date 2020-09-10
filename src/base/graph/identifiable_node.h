#ifndef BASE__IDENTIFIABLE_NODE_H_
#define BASE__IDENTIFIABLE_NODE_H_

#include "base/graph/graph_object.h"

class IdentifiableNode : public GraphObject {
public:
    const std::string id;

    IdentifiableNode(const std::string id)
        : id(id) { }
    ~IdentifiableNode() = default;

    std::string to_string() const override {
        return id;
    }

    ObjectType type() const override {
        return ObjectType::identifiable_node;
    }

    bool operator==(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::identifiable_node) {
            const auto& casted_rhs = static_cast<const IdentifiableNode&>(rhs);
            return this->id == casted_rhs.id;
        }
        else return false;
    }

    bool operator!=(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::identifiable_node) {
            const auto& casted_rhs = static_cast<const IdentifiableNode&>(rhs);
            return this->id != casted_rhs.id;
        }
        else return true;
    }

    bool operator<=(const GraphObject&) const override {
        return false;
    }

    bool operator>=(const GraphObject&) const override {
        return false;
    }

    bool operator<(const GraphObject&) const override {
        return false;
    }

    bool operator>(const GraphObject&) const override {
        return false;
    }
};

#endif // BASE__IDENTIFIABLE_NODE_H_
