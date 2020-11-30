#include "binding_where.h"

#include <cassert>

#include "storage/index/bplus_tree/bplus_tree.h"

using namespace std;

BindingWhere::BindingWhere(GraphModel& model, Binding& child_binding, VarId max_var_id_in_child,
                           std::map<VarId, std::pair<VarId, ObjectId>> property_map) :
    model               (model),
    max_var_id_in_child (max_var_id_in_child),
    child_binding       (child_binding),
    property_map        (move(property_map)) { }


std::string BindingWhere::to_string() const {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


void BindingWhere::clear_cache() {
    cache.clear();
}


std::shared_ptr<GraphObject> BindingWhere::operator[](VarId var_id) {
    if (var_id <= max_var_id_in_child) {
        return child_binding[var_id];
    } else {
        auto cache_find = cache.find(var_id);
        if (cache_find != cache.end()) {
            return (*cache_find).second;
        } else {
            auto find_property = property_map.find(var_id);
            assert(find_property != property_map.end());

            auto property_var_id  = find_property->second.first;
            auto property_key = find_property->second.second;
            auto var = child_binding[property_var_id];

            // TODO: if optionals were allowed, should check is not null
            auto value = model.get_property_value(*var, property_key);
            cache.insert({ var_id, value });
            return value;
        }
    }
}


ObjectId BindingWhere::get_id(const VarId var_id) {
    // TODO: Ver el caso con los VarId fuera del match
    return child_binding.get_id(var_id);
}

