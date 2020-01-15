#include "binding_id.h"

#include "file/index/object_file/object_file.h"

using namespace std;

BindingId::BindingId(int_fast32_t var_count)
{
    // TODO: if CHECKING flag is set, check var_count >= 0
    dict = vector<ObjectId>(var_count);
}


int_fast32_t BindingId::var_count() {
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
        if (!other.dict[i].is_null()) {
            this->dict[i] = other.dict[i];
        }
    }
}

void BindingId::add(VarId var_id, ObjectId id) {
    dict[var_id] = id;
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


ObjectId BindingId::operator[](VarId var_id) {
    return dict[var_id];
}

// BindingId::iterator BindingId::begin() {
//     return iterator(dict);
// }

void BindingId::print(vector<string>& var_names) const
{
    std::cout << "BindingId: ";
    for (size_t i = 0; i < dict.size(); ++i) {
        std::cout << var_names[i] << ":[" << (dict[i] >> 56) << "]" << (dict[i] & 0x00FFFFFFFFFFFFFFUL) << "\t";
    }
    std::cout << endl;
}

void BindingId::print(vector<string>& var_names, ObjectFile& object_file) const
{
    std::cout << "BindingId: ";
    for (size_t i = 0; i < dict.size(); ++i) {
        std::cout << var_names[i] << ": ";
        if (var_names[i].find('.') != std::string::npos) { // if has a point is a value
            // TODO: assuming value is a string
            auto bytes = object_file.read(dict[i]);
            string value_string(bytes->begin(), bytes->end());
            cout << value_string << "\t";
        }
        else { // else is a Element (Node or Edge)
            cout << dict[i] << "\t";
        }
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
