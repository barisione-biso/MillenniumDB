#ifndef BASE__ANONYMOUS_NODE_H_
#define BASE__ANONYMOUS_NODE_H_

class AnonymousNode {
public:
    uint64_t id;

    AnonymousNode(const uint64_t id)
        : id(id) { }
    ~AnonymousNode() = default;

    inline void operator=(const AnonymousNode& other) {
        id = other.id;
    }

    inline bool operator==(const AnonymousNode& rhs) const noexcept {
        return this->id == rhs.id;
    }

    inline bool operator!=(const AnonymousNode& rhs) const noexcept {
        return this->id != rhs.id;
    }

    inline bool operator<=(const AnonymousNode& rhs) const noexcept {
        return this->id <= rhs.id;
    }

    inline bool operator>=(const AnonymousNode& rhs) const noexcept {
        return this->id >= rhs.id;
    }

    inline bool operator<(const AnonymousNode& rhs) const noexcept {
        return this->id < rhs.id;
    }

    inline bool operator>(const AnonymousNode& rhs) const noexcept {
        return this->id > rhs.id;
    }
};

#endif // BASE__ANONYMOUS_NODE_H_
