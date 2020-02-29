#ifndef BASE__OBJECT_ID_H_
#define BASE__OBJECT_ID_H_

#include <memory>

#define NULL_OBJECT_ID UINT64_MAX
#define NOT_FOUND_OBJECT_ID UINT64_MAX - 1

class ObjectFile;
class Value;

class ObjectId {

public:
    uint64_t id;

    ObjectId(uint64_t id);
    ObjectId(); // initialized with NULL_OBJECT_ID
    ~ObjectId() = default;

    bool is_null();
    bool not_found();

    static ObjectId get_null();
    static ObjectId get_not_found();

    operator uint64_t() const;

    void operator = (const ObjectId& other);
    bool operator ==(const ObjectId& rhs) const;
    bool operator !=(const ObjectId& rhs) const;

};

#endif //BASE__OBJECT_ID_H_
