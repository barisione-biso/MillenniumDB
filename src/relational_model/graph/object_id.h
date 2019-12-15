#ifndef RELATIONAL_MODEL__OBJECT_ID_H_
#define RELATIONAL_MODEL__OBJECT_ID_H_

#include <iostream>

class ObjectId {
public:
    ObjectId(int graph_id, uint64_t id)
        : graph_id(graph_id), id(id){}
    ~ObjectId() = default;

    int graph_id;
    uint64_t id;

    operator uint64_t() const { return id; }

    void operator=(const ObjectId& other) {
        this->graph_id = other.graph_id;
        this->id = other.id;
    }

    bool operator ==(const ObjectId& rhs) const {
        return id == rhs.id && graph_id == rhs.graph_id;
    }

    bool operator !=(const ObjectId& rhs) const {
        return id != rhs.id || graph_id != rhs.graph_id;
    }
};

#endif //RELATIONAL_MODEL__OBJECT_ID_H_
