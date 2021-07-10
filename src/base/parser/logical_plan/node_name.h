#ifndef BASE__NODE_NAME_H_
#define BASE__NODE_NAME_H_

#include <ostream>
#include <string>

class NodeName {
public:
    const std::string value;

    explicit NodeName(std::string value)
        : value(std::move(value)) { }

    ~NodeName() = default;

    inline bool operator<(const NodeName& rhs) const noexcept {
        return value < rhs.value;
    }

    inline bool operator<=(const NodeName& rhs) const noexcept {
        return value <= rhs.value;
    }

    inline bool operator>(const NodeName& rhs) const noexcept {
        return value > rhs.value;
    }

    inline bool operator>=(const NodeName& rhs) const noexcept {
        return value >= rhs.value;
    }

    inline bool operator==(const NodeName& rhs) const noexcept {
        return value == rhs.value;
    }

    inline bool operator!=(const NodeName& rhs) const noexcept {
        return value != rhs.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const NodeName& node_name) {
        return os << node_name.value;
    }
};

#endif // BASE__NODE_NAME_H_
