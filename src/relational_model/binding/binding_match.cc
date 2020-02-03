#include "binding_match.h"

#include "file/index/object_file/object_file.h"
#include "relational_model/binding/binding_id.h"

#include <iostream>

using namespace std;

BindingMatch::BindingMatch(ObjectFile& object_file, map<string, VarId>& var_pos, unique_ptr<BindingId> binding_id)
    : object_file(object_file), var_pos(var_pos), binding_id(move(binding_id)) { }


void BindingMatch::print() const {
    cout << "BindingMatch should not be called to be printed.\n";
}


shared_ptr<Value> BindingMatch::operator[](const string& var) {
    // search in the cache map
    auto cache_search = cache.find(var);
    if (cache_search != cache.end()) { // Found in the cache
        return (*cache_search).second;
    }
    else {                             // Not found in the cache
        // search in the
        auto var_pos_search = var_pos.find(var);
        if (var_pos_search != var_pos.end()) {
            auto var_id = (*var_pos_search).second;
            auto object_id = (*binding_id)[var_id];
            return object_id.get_value(object_file);
        }
        else return nullptr;
    }
}
