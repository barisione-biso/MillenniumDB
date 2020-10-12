#ifndef BASE__ANONYMOUS_NODE_H_
#define BASE__ANONYMOUS_NODE_H_

#include "base/graph/graph_object.h"

class AnonymousNode : public GraphObject {
public:
    const uint64_t id;

    AnonymousNode(const uint64_t id)
        : id(id) { }
    ~AnonymousNode() = default;

    std::string to_string() const noexcept override {
        return "AnonymousObject(" + std::to_string(id) + ")";
    }

    ObjectType type() const noexcept override {
        return ObjectType::anonymous_node;
    }

    bool operator==(const GraphObject& rhs) const noexcept override {
        if (rhs.type() == ObjectType::anonymous_node) {
            const auto& casted_rhs = static_cast<const AnonymousNode&>(rhs);
            return this->id == casted_rhs.id;
        }
        else return false;
    }

    bool operator!=(const GraphObject& rhs) const noexcept override {
        if (rhs.type() == ObjectType::anonymous_node) {
            const auto& casted_rhs = static_cast<const AnonymousNode&>(rhs);
            return this->id != casted_rhs.id;
        }
        else return true;
    }

    bool operator<=(const GraphObject&) const noexcept override {
        return false;
    }

    bool operator>=(const GraphObject&) const noexcept override {
        return false;
    }

    bool operator<(const GraphObject&) const noexcept override {
        return false;
    }

    bool operator>(const GraphObject&) const noexcept override {
        return false;
    }
};

#endif // BASE__ANONYMOUS_NODE_H_
