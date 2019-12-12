#include "binding_id_range.h"

using namespace std;

BindingIdRange::BindingIdRange() {
    dict = make_unique<map<VarId, tuple<ObjectId,ObjectId>>>();
}


void BindingIdRange::add(unique_ptr<BindingIdRange::Iter> iter) {
    auto next = iter->next();
    while (next != nullptr) {
        auto min_max = next->second;
        add(next->first, min_max.first, min_max.second);
        next = iter->next();
    }
}

void BindingIdRange::add(VarId var_id, ObjectId min_id, ObjectId max_id) {
    dict->insert(make_pair(var_id, make_pair(min_id, max_id)));
}


void BindingIdRange::try_add(unique_ptr<BindingIdRange::Iter> iter) {
    auto next = iter->next();
    while (next != nullptr) {
        auto min_max = next->second;
        try_add(next->first, min_max.first, min_max.second);
        next = iter->next();
    }
}


void BindingIdRange::try_add(VarId var_id, ObjectId min_id, ObjectId max_id) {
    if (dict->find(var_id) == dict->end()) {
        dict->insert(make_pair(var_id, make_pair(min_id, max_id)));
    }
}


// ObjectId const* BindingIdRange::search_id(VarId var_id) const {
//     auto it = dict->find(var_id);
//     if (it == dict->end()) {
//         return nullptr;
//     }
//     else {
//         return &(it->second);
//     }
// }

unique_ptr<BindingIdRange::Iter> BindingIdRange::get_values() const {
    return make_unique<Iter>(*dict);
}

// void BindingId::print(map<int, string>& var_names) const
// {
//     std::cout << "BindingId: ";
//     auto it = dict->cbegin();
//     while (it != dict->cend()) {
//         std::cout << var_names[it->first.var_id] << ":[" << ((it->second.id) >> 56) << "]" << (it->second.id & 0x00FFFFFFFFFFFFFFUL) << "\t\t";
//         it++;
//     }
//     std::cout << endl;
// }

/******************************* Iter ********************************/
BindingIdRange::Iter::Iter(const map<VarId, pair<ObjectId, ObjectId>>& dict)
    : dict(dict)
{
    it = dict.cbegin();
}

pair<VarId, pair<ObjectId,ObjectId>> const* BindingIdRange::Iter::next()
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
