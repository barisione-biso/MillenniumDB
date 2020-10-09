#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <iostream>
#include <iterator>
#include <vector>

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"

class BindingId {

private:
    std::vector<ObjectId> object_ids;

public:
    BindingId(int_fast32_t var_count);
    ~BindingId() = default;

    void add_all(BindingId&);
    void add(VarId, ObjectId);
    void print() const;

    inline ObjectId operator[](VarId var_id) const noexcept {
        return object_ids[var_id.id];
    }

    inline int_fast32_t var_count() const noexcept {
        return object_ids.size();
    }
};


#endif // RELATIONAL_MODEL__BINDING_ID_H_
