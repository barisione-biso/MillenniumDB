#include "binding_match.h"

#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

BindingMatch::BindingMatch(GraphModel& model, const map<string, VarId>& var_pos, BindingId* binding_id) :
    model      (model),
    var_pos    (var_pos),
    binding_id (binding_id) { }


BindingMatch::~BindingMatch() = default;


std::string BindingMatch::to_string() const {
    string result;
    result.reserve(128);
    result += "{ ";
    bool first = true;
    for (auto&& [var, varid] : var_pos) {
        if (first) {
            first = false;
        } else {
            result += ", ";
        }
        auto obj = model.get_graph_object((*binding_id)[varid]);
        result += var;
        result += ':';
        result += obj->to_string();
    }
    result += " }\n";
    return result;
}


shared_ptr<GraphObject> BindingMatch::operator[](const string& var) {
    // search in the cache map
    // auto cache_search = cache.find(var);
    // if (cache_search != cache.end()) { // Found in the cache
    //     return (*cache_search).second;
    // }
    // else {                             // Not found in the cache
        // search in the binding_id
        auto var_pos_search = var_pos.find(var);
        if (var_pos_search != var_pos.end()) {
            auto var_id = (*var_pos_search).second;
            auto object_id = (*binding_id)[var_id];
            auto value = model.get_graph_object(object_id);
            // cache.insert({ var, value });
            return value;
        }
        else return nullptr;
    // }
}


shared_ptr<GraphObject> BindingMatch::get(const string& var, const string& key) {
    return (*this)[var + "." + key];
}
