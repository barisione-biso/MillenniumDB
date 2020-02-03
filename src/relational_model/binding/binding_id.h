#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <iostream>
#include <iterator>
#include <vector>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

class ObjectFile;


class BindingId {

private:
    std::vector<ObjectId> dict;

public:
    BindingId(int_fast32_t var_count);
    ~BindingId() = default;

    ObjectId operator[](VarId);

    int_fast32_t var_count();
    void add_all(BindingId&);
    void add(VarId, ObjectId);

    void print(std::vector<std::string>& var_names) const;
    void print(std::vector<std::string>& var_names, ObjectFile& object_file) const;
};


#endif //RELATIONAL_MODEL__BINDING_ID_H_
