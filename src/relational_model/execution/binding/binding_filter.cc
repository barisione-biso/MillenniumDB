#include "binding_filter.h"

#include <cassert>

#include "storage/index/bplus_tree/bplus_tree.h"

using namespace std;

BindingFilter::BindingFilter(GraphModel& model, Binding& binding) :
    model   (model),
    binding (binding) { }


std::string BindingFilter::to_string() const {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


std::shared_ptr<GraphObject> BindingFilter::operator[](const std::string& var_name) {
    return binding[var_name];
}


std::shared_ptr<GraphObject> BindingFilter::get(const std::string& var_name, const std::string& key) {
    auto search_var = var_name + "." + key;
    auto value = binding[search_var];
    if (value != nullptr) {
        return value;
    }
    auto search = cache.find(search_var);
    if (search != cache.end()) {
        return (*search).second;
    } else { // not present in the cache nor the original binding
        const auto var = binding[var_name];
        assert(var != nullptr && ("var not in binding"));
        auto value = model.get_property_value(*var, key);
        cache.insert({ search_var, value });
        return value;
    }
}
