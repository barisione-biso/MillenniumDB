#ifndef BASE__BINDING_ID_H_
#define BASE__BINDING_ID_H_

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"

class BindingId {
private:
    std::size_t size;

public:
    // TODO CRIS: Mover object_ids a private
    ObjectId* object_ids; // array
    BindingId(std::size_t size) :
        size       (size),
        object_ids (new ObjectId[size]())
    {
        for (std::size_t i = 0; i < size; ++i) {
            assert(object_ids[i].is_null());
        }
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

    inline ObjectId operator[](VarId var_id) const noexcept {
        return object_ids[var_id.id];
    }

    inline std::size_t var_count() const noexcept {
        return size;
    }

    inline void print() const noexcept{
        printf("printing binding\n");
        for(unsigned int i = 0; i < size; i++)
        {
            auto type = object_ids[i].id & 0xFF00'0000'0000'0000;
            type >>= 56;
            auto id  = object_ids[i].id & 0x00FF'FFFF'FFFF'FFFF;
            printf("type: %ld, id: %ld\n", type, id);
        }
    }
};


#endif // BASE__BINDING_ID_H_
