#ifndef RELATIONAL_MODEL__BINDING_ID_RANGE_H_
#define RELATIONAL_MODEL__BINDING_ID_RANGE_H_

#include <iostream>
#include <iterator>
#include <map>
#include <memory>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

using namespace std;

class BindingIdRange {
public:
    class Iter;
    BindingIdRange();
    // BindingId(unique_ptr<map<VarId, ObjectId>> dict);
    ~BindingIdRange() = default;

    void add(unique_ptr<BindingIdRange::Iter>);
    void add(VarId, ObjectId min, ObjectId max);
    void try_add(unique_ptr<BindingIdRange::Iter>);
    void try_add(VarId, ObjectId min, ObjectId max);

    pair<ObjectId, ObjectId> const* search_id(VarId var_name) const;
    unique_ptr<BindingIdRange::Iter> get_values() const;
    // void print(map<int, string>& var_names) const;

    class Iter {
        public:
            Iter(const map<VarId, pair<ObjectId, ObjectId>>& dict);
            ~Iter() = default;
            pair<VarId, pair<ObjectId, ObjectId>> const* next();

        private:
            map<VarId, pair<ObjectId, ObjectId>>::const_iterator it;
            map<VarId, pair<ObjectId, ObjectId>> const& dict;
    };

private:
    unique_ptr<map<VarId, pair<ObjectId, ObjectId>>> dict;
};


#endif //RELATIONAL_MODEL__BINDING_ID_RANGE_H_