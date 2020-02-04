#ifndef RELATIONAL_MODEL__OBJECT_ID_H_
#define RELATIONAL_MODEL__OBJECT_ID_H_

#include <memory>

#define NULL_OBJECT_ID UINT64_MAX
#define NOT_FOUND_OBJECT_ID UINT64_MAX - 1

class ObjectFile;
class Value;

class ObjectId {

private:
    uint64_t id;

public:
    ObjectId(uint64_t id);
    ObjectId(); // initialized with NULL_OBJECT_ID
    ~ObjectId() = default;

    bool is_null();
    bool not_found();

    static ObjectId get_null();
    static ObjectId get_not_found();

    std::shared_ptr<Value> get_value(ObjectFile& object_file);
    operator uint64_t() const;

    void operator = (const ObjectId& other);
    bool operator ==(const ObjectId& rhs) const;
    bool operator !=(const ObjectId& rhs) const;

};

#endif //RELATIONAL_MODEL__OBJECT_ID_H_
