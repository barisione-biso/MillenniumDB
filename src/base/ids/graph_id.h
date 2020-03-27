#ifndef BASE__GRAPH_ID_H_
#define BASE__GRAPH_ID_H_

#include <cstdint>

class GraphId {
public:
    uint_fast32_t id;

    GraphId()
        : id(0) { }
    GraphId(uint_fast32_t id)
        : id(id) { }
    ~GraphId() = default;

    operator uint_fast32_t() const {
        return id;
    }

    bool operator<(const GraphId other) const {
        return this->id < other.id;
    }
};

#endif //BASE__GRAPH_ID_H_
