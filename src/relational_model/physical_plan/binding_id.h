#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <iostream>
#include <iterator>
#include <vector>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

using namespace std;

class BindingId {
    private:
        vector<ObjectId> dict;
    public:
        class iterator;
        BindingId(uint_fast32_t var_count);
        ~BindingId() = default;

        ObjectId operator[](VarId);

        uint_fast32_t var_count();
        void add_all(BindingId&);
        void add(VarId, ObjectId);

        void print(vector<string>& var_names) const;

};


#endif //RELATIONAL_MODEL__BINDING_ID_H_