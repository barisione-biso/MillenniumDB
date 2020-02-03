#include "object_id.h"

#include "base/graph/value/value_string.h"
#include "file/index/object_file/object_file.h"

using namespace std;

ObjectId::ObjectId(uint64_t id)
    : id(id) { }


ObjectId::ObjectId()
    : id(NULL_OBJECT_ID) { }


bool ObjectId::is_null() {
    return id == NULL_OBJECT_ID;
}


bool ObjectId::not_found() {
    return id == NOT_FOUND_OBJECT_ID;
}


ObjectId ObjectId::get_null() {
    return ObjectId(NULL_OBJECT_ID);
}


ObjectId ObjectId::get_not_found() {
    return ObjectId(NOT_FOUND_OBJECT_ID);
}


ObjectId::operator uint64_t() const {
    return id;
}


shared_ptr<Value> ObjectId::get_value(ObjectFile& object_file) { // TODO: mover a relational graph?
    auto bytes = object_file.read(id);
    string value_string(bytes->begin(), bytes->end());
    return make_shared<ValueString>(move(value_string)); // TODO: revisar prefijo y soportar todos los tipos de valores
}


void ObjectId::operator=(const ObjectId& other) {
    this->id = other.id;
}


bool ObjectId::operator==(const ObjectId& rhs) const {
    return id == rhs.id;
}


bool ObjectId::operator!=(const ObjectId& rhs) const {
    return id != rhs.id;
}