#ifndef BASE__EDGE_H_
#define BASE__EDGE_H_

#include "base/graph/graph_object.h"

class Edge : public GraphObject {
public:
    const uint64_t id;

    Edge(const uint64_t id)
        : id(id) { }
    ~Edge() = default;

    inline std::string to_string() const noexcept override {
        return "_e" + std::to_string(id);
    }

    ObjectType type() const noexcept override {
        return ObjectType::edge;
    }

    bool operator==(const GraphObject& rhs) const noexcept override {
        if (rhs.type() == ObjectType::edge) {
            const auto& casted_rhs = static_cast<const Edge&>(rhs);
            return this->id == casted_rhs.id;
        }
        else return false;
    }

    bool operator!=(const GraphObject& rhs) const noexcept override {
        if (rhs.type() == ObjectType::edge) {
            const auto& casted_rhs = static_cast<const Edge&>(rhs);
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

#endif // BASE__EDGE_H_
