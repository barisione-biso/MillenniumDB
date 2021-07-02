#ifndef BASE__NODE_ID_H_
#define BASE__NODE_ID_H_

#include <ostream>
#include <string>
#include <variant>

struct NodeIdToStringVisitor {
    NodeIdToStringVisitor() { }

    std::string operator()(const std::string& s) {
        return s;
    }

    std::string operator()(const int64_t i) {
        return std::to_string(i);
    }

    std::string operator()(const bool b) {
        return b ? "true" : "false";
    }

    std::string operator()(const float f) {
        return std::to_string(f);
    }
};

class NodeId {
public:
    // std::variant<bool, int64_t, float, std::string> value;
    std::variant</*Var, Identifier, */std::string, bool, int64_t, float> value;

    NodeId(std::string _value) :
        value (_value)
    {
        assert(!_value.empty());
    }

    NodeId(bool _value) :
        value (_value) { }

    NodeId(int64_t _value) :
        value (_value) { }

    NodeId(float _value) :
        value (_value) { }

    ~NodeId() = default;

    inline bool operator==(const NodeId& rhs) const noexcept { return value == rhs.value; }
    inline bool operator!=(const NodeId& rhs) const noexcept { return value != rhs.value; }
    inline bool operator< (const NodeId& rhs) const noexcept { return value <  rhs.value; }
    inline bool operator<=(const NodeId& rhs) const noexcept { return value <= rhs.value; }
    inline bool operator> (const NodeId& rhs) const noexcept { return value >  rhs.value; }
    inline bool operator>=(const NodeId& rhs) const noexcept { return value >= rhs.value; }

    inline bool is_var() const {
        return std::holds_alternative<std::string>(value) && std::get<std::string>(value)[0] == '?';
    }

    inline std::string to_string() const {
        return std::visit(NodeIdToStringVisitor(), value);
    }

    friend std::ostream& operator<<(std::ostream& os, const NodeId& node_id) {
        return os << node_id.to_string();
    }
};

#endif // BASE__NODE_ID_H_
