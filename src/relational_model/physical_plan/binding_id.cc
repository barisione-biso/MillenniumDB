#include "binding_id.h"

using namespace std;

BindingId::BindingId()
{
    dict = make_unique<map<VarId, ObjectId>>();
}

void BindingId::add(unique_ptr<BindingId::Iter> iter)
{
    auto next = iter->next();
    while (next != nullptr) {
        add(next->first, next->second);
        next = iter->next();
    }
}

void BindingId::add(VarId var_id, ObjectId object_id)
{
    dict->insert(make_pair(var_id, object_id));
}

void BindingId::try_add(unique_ptr<BindingId::Iter> iter)
{
    auto next = iter->next();
    while (next != nullptr) {
        try_add(next->first, next->second);
        next = iter->next();
    }
}

void BindingId::try_add(VarId var_id, ObjectId object_id)
{
    if (dict->find(var_id) == dict->end()) {
        dict->insert(make_pair(var_id, object_id));
    }
}

ObjectId const* BindingId::search_id(VarId var_id) const
{
    auto it = dict->find(var_id);
    if (it == dict->end()) {
        return nullptr;
    }
    else {
        return &(it->second);
    }
}

unique_ptr<BindingId::Iter> BindingId::get_values() const
{
    return make_unique<Iter>(*dict);
}

void BindingId::print(map<int, string>& var_names) const
{
    cout << "BindingId: ";
    auto it = dict->cbegin();
    while (it != dict->cend()) {
        cout << ">";
        cout << var_names[it->first.var_id] << ":" << it->second.id << "\t";
        it++;
    }
    cout << endl;
}

/******************************* Iter ********************************/
BindingId::Iter::Iter(const map<VarId, ObjectId>& dict)
    : dict(dict)
{
    it = dict.cbegin();
}

pair<VarId const, ObjectId> const* BindingId::Iter::next()
{
    if (it != dict.cend()) {
        auto res = &(*it);
        it++;
        return res;
    }
    else {
        return nullptr;
    }
}
