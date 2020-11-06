#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <cstring>
#include <iostream>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"

class BindingId {

private:
    ObjectId* object_ids; // array
    std::size_t size;

public:
    BindingId(std::size_t size) :
        size(size)
    {
        object_ids = new ObjectId[size];
    }

    ~BindingId() {
        delete[] object_ids;
    }

    inline void add_all(BindingId& other) noexcept {
        std::memcpy(this->object_ids, other.object_ids, sizeof(ObjectId) * other.size);
    }

    inline void add(VarId var_id, ObjectId obj_id) noexcept {
        object_ids[var_id.id] = obj_id;
    }

    // inline void print() const noexcept {
    //     std::cout << "BindingId: ";
    //     for (std::size_t i = 0; i < size; ++i) {
    //         std::cout << i << ":[" << (object_ids[i].id >> 56) << "]" << (object_ids[i].id & 0x00'0000'FFFFFFFFFFUL) << "\t";
    //     }
    //     std::cout << "\n";
    // }

    inline ObjectId operator[](VarId var_id) const noexcept {
        return object_ids[var_id.id];
    }

    inline std::size_t var_count() const noexcept {
        return size;
    }
};


#endif // RELATIONAL_MODEL__BINDING_ID_H_
