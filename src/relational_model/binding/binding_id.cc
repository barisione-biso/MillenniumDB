#include "binding_id.h"

#include "file/index/object_file/object_file.h"

using namespace std;

BindingId::BindingId(int_fast32_t var_count) {
    dict = vector<ObjectId>(var_count);
}


int_fast32_t BindingId::var_count() {
    return dict.size();
}


void BindingId::add_all(BindingId& other) {
    for (size_t i = 0; i < other.dict.size(); ++i) {
        if (!other.dict[i].is_null()) {
            this->dict[i] = other.dict[i];
        }
    }
}


void BindingId::add(VarId var_id, ObjectId id) {
    dict[var_id] = id;
}


ObjectId BindingId::operator[](VarId var_id) {
    return dict[var_id];
}


void BindingId::print(vector<string>& var_names) const {
    std::cout << "BindingId: ";
    for (size_t i = 0; i < dict.size(); ++i) {
        std::cout << var_names[i] << ":[" << (dict[i] >> 56) << "]" << (dict[i] & 0x00FFFFFFFFFFFFFFUL) << "\t";
    }
    std::cout << endl;
}


void BindingId::print(vector<string>& var_names, ObjectFile& object_file) const {
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
