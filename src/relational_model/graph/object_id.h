#ifndef RELATIONAL_MODEL__OBJECT_ID_H_
#define RELATIONAL_MODEL__OBJECT_ID_H_

#include <iostream>

class ObjectId {
public:
    ObjectId(int graph_id, u_int64_t id)
        : graph_id(graph_id), id(id){}
    ~ObjectId() = default;

    const int graph_id;
    const u_int64_t id;

    bool operator ==(const ObjectId& rhs) const {
        return id == rhs.id && graph_id == rhs.graph_id;
    }

    bool operator !=(const ObjectId& rhs) const {
        return id != rhs.id || graph_id != rhs.graph_id;
    }
};

#endif //RELATIONAL_MODEL__OBJECT_ID_H_
