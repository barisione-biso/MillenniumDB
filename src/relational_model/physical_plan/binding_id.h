#ifndef RELATIONAL_MODEL__BINDING_ID_H_
#define RELATIONAL_MODEL__BINDING_ID_H_

#include <iostream>
#include <iterator>
#include <map>
#include <memory>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

using namespace std;

class BindingId {
public:
    class Iter;
    BindingId();
    // BindingId(unique_ptr<map<VarId, ObjectId>> dict);
    ~BindingId() = default;

    void add(unique_ptr<BindingId::Iter>);
    void add(VarId, ObjectId);
    void try_add(unique_ptr<BindingId::Iter>);
    void try_add(VarId, ObjectId);

    ObjectId const* search_id(VarId var_name) const;
    unique_ptr<BindingId::Iter> get_values() const;
    void print(map<int, string>& var_names) const;

    class Iter {
        public:
            Iter(const map<VarId, ObjectId>& dict);
            ~Iter() = default;
            pair<VarId const, ObjectId> const* next();

        private:
            map<VarId, ObjectId>::const_iterator it;
            map<VarId, ObjectId> const& dict;
    };

private:
    unique_ptr<map<VarId, ObjectId>> dict;
};


#endif //RELATIONAL_MODEL__BINDING_ID_H_