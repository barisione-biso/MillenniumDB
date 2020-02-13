#ifndef BASE__NODE_H_
#define BASE__NODE_H_

 #include <cstdint>

class Node {
private:
    uint64_t id;

public:
    Node(uint64_t id)
        : id(id) { }
    ~Node() = default;

    uint64_t get_id() { return id; }
};

#endif //BASE__NODE_H_
