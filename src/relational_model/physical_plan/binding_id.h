#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <iostream>
#include <iterator>
#include <vector>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

using namespace std;

class IdRange {
    public:
        IdRange(ObjectId min, ObjectId max)
            : min(min), max(max) { }
        IdRange()
            : min(ObjectId(UINT64_MAX)), max(ObjectId(UINT64_MAX))
        { }
        ~IdRange() = default;
        ObjectId min;
        ObjectId max;

        void operator=(const IdRange& other) {
            this->min = other.min;
            this->max = other.max;
        }
        bool unbinded() {
            return (uint64_t) min == UINT64_MAX;
        }
};

class BindingId {
    private:
        vector<IdRange> dict;
    public:
        class iterator;
        BindingId(uint_fast32_t var_count);
        ~BindingId() = default;

        IdRange operator[](VarId);

        uint_fast32_t var_count();
        void add_all(BindingId&);
        void add(VarId, ObjectId min, ObjectId max);

        void print(vector<string>& var_names) const;

};


#endif //RELATIONAL_MODEL__BINDING_ID_H_