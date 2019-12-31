#ifndef RELATIONAL_MODEL__OBJECT_ID_H_
#define RELATIONAL_MODEL__OBJECT_ID_H_

#include <iostream>

#define NULL_OBJECT_ID UINT64_MAX
#define NOT_FOUND_OBJECT_ID UINT64_MAX - 1

class ObjectId {
private:
    uint64_t id;
public:
    ObjectId(uint64_t id)
        : id(id){}
    ObjectId()
        : id(NULL_OBJECT_ID){}
    ~ObjectId() = default;

    bool is_null() { return id == NULL_OBJECT_ID; }
    bool not_found() { return id == NOT_FOUND_OBJECT_ID; }

    static ObjectId get_null() {
        return ObjectId(NULL_OBJECT_ID);
    }

    operator uint64_t() const { return id; }

    void operator=(const ObjectId& other) {
        this->id = other.id;
    }

    bool operator ==(const ObjectId& rhs) const {
        return id == rhs.id;
    }

    bool operator !=(const ObjectId& rhs) const {
        return id != rhs.id;
    }

};

#endif //RELATIONAL_MODEL__OBJECT_ID_H_
