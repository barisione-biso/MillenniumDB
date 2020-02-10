#include "binding_match.h"

#include "file/index/object_file/object_file.h"
#include "relational_model/binding/binding_id.h"
#include "relational_model/physical_plan/binding_id_iter/graph_scan.h"

#include <iostream>

using namespace std;

BindingMatch::BindingMatch(ObjectFile& object_file, map<string, VarId>& var_pos, unique_ptr<BindingId> binding_id)
    : object_file(object_file), var_pos(var_pos), binding_id(move(binding_id)) { }


void BindingMatch::print() const {
    cout << "BindingMatch should not be called to be printed.\n";
    for (auto&& [a, b] : var_pos) {
        cout << a << ", " << b.id << "\n";
    }
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


shared_ptr<Value> BindingMatch::try_extend(const string& var, const string& key) {
    auto var_pos_search = var_pos.find(var);
    if (var_pos_search != var_pos.end()) {
        /*auto element_var_id = (*var_pos_search).second;
        auto element_object_id = (*binding_id)[element_var_id];

        auto key_object_id = graph.get_id(key);
        // TODO: search in EKV / NKV, need to differientate
        vector<pair<ObjectId, int>> terms;
        vector<pair<VarId, int>> vars;

        terms.push_back(make_pair(element_object_id, 0));
        terms.push_back(make_pair(key_object_id, 1));
        vars.push_back(make_pair(var_pos[var + "." + key], 2)); // TODO: que hacer espacio porque el array ya esta lleno

        GraphScan scan = GraphScan(bpt, terms, vars);*/
        return nullptr;
    }
    else {
        return nullptr;
    }
}
