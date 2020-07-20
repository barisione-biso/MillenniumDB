#ifndef BASE__NODE_H_
#define BASE__NODE_H_

#include "base/graph/graph_object.h"

class Node : public GraphObject {
public:
    // const uint_fast16_t graph;
    const uint64_t id;

    Node(uint64_t id)
        : id(id) { }
    ~Node() = default;

    std::string to_string() const override {
        auto graph = (id >> 40) & 0xFFFF;
        auto unmasked_id = id & 0x0000'00FF'FFFF'FFFFUL;
        return "Node(" + std::to_string(graph) + ", " + std::to_string(unmasked_id) + ")";
    }

    ObjectType type() const override {
        return ObjectType::node;
    }

    bool operator==(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::node) {
            const auto& casted_rhs = static_cast<const Node&>(rhs);
            return this->id == casted_rhs.id;
        }
        else return false;
    }

    bool operator!=(const GraphObject& rhs) const override {
        if (rhs.type() == ObjectType::node) {
            const auto& casted_rhs = static_cast<const Node&>(rhs);
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

#endif // BASE__NODE_H_
