#ifndef BASE__EDGE_H_
#define BASE__EDGE_H_

class Edge {
private:
    uint64_t id;

public:
    Edge(uint64_t id)
        : id(id) { }
    ~Edge() = default;

    uint64_t get_id() { return id; }
};

#endif //BASE__EDGE_H_
