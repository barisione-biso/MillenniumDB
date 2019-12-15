#include "binding_id.h"

using namespace std;

BindingId::BindingId(uint_fast32_t var_count)
    // : dict()
{
    dict = vector<IdRange>(var_count);
}


uint_fast32_t BindingId::var_count() {
    return dict.size();
}


void BindingId::add_all(BindingId& other) {
    // auto next = iter.next();
    // while (next != nullptr) {
    //     auto range = next->second;
    //     add(next->first, range.min, range.max);
    //     next = iter.next();
    // }
    for (size_t i = 0; i < other.dict.size(); ++i) {
        if (!other.dict[i].unbinded()) {
            this->dict[i] = other.dict[i];
        }
    }
}

void BindingId::add(VarId var_id, ObjectId min_id, ObjectId max_id) {
    dict[var_id.var_id] = IdRange(min_id, max_id);
}


// void BindingId::try_add_all(BindingId::iterator& iter) {
    // auto next = iter.next();
    // while (next != nullptr) {
    //     auto range = next->second;
    //     try_add(next->first, range.min, range.max);
    //     next = iter.next();
    // }
// }


// void BindingId::try_add(VarId var_id, ObjectId min_id, ObjectId max_id) { // TODO: sera mas facil simplemente sobreescribir?
//     if (dict[var_id.var_id].unbinded()) {
//         dict[var_id.var_id] = IdRange(min_id, max_id);
//     }
// }


IdRange BindingId::operator[](VarId var_id) {
    return dict[var_id.var_id];
}

// BindingId::iterator BindingId::begin() {
//     return iterator(dict);
// }

void BindingId::print(vector<string>& var_names) const
{
    std::cout << "BindingId: ";
    // auto it = dict.begin();
    // while (it != dict.end()) {
    //     std::cout << var_names[it->first.var_id] << ":[" << ((it->second.min.id) >> 56) << "]" << (it->second.min.id & 0x00FFFFFFFFFFFFFFUL);
    //     std::cout << "~[" << ((it->second.max.id) >> 56) << "]" << (it->second.max.id & 0x00FFFFFFFFFFFFFFUL) << "\t";
    //     it++;
    // }
    for (size_t index = 0; index < dict.size(); ++index) {
        std::cout << var_names[index] << ":[" << (dict[index].min.id >> 56) << "]" << (dict[index].min.id & 0x00FFFFFFFFFFFFFFUL);
        std::cout << "~[" << (dict[index].max.id >> 56) << "]" << (dict[index].max.id & 0x00FFFFFFFFFFFFFFUL) << "\t";
    }
    std::cout << endl;
}

/******************************* iterator ********************************/
// BindingId::iterator::iterator(vector<IdRange>& dict)
//     : dict(dict)
// {
//     it = dict.cbegin();
// }

// const pair<const VarId, IdRange>* BindingId::iterator::next() {
//     if (it != dict.cend()) {
//         auto res = &(*it);
//         it++;
//         return res;
//     }
//     else {
//         return nullptr;
//     }
// }
