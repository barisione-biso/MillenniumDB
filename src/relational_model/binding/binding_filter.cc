#include "binding_filter.h"

#include "relational_model/relational_model.h"
#include "relational_model//graph/relational_graph.h"

using namespace std;

BindingFilter::BindingFilter(Binding& binding, map<string, pair<GraphId, ObjectType>>& var_info)
    : binding(binding), var_info(var_info) { }


void BindingFilter::print() const {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


std::shared_ptr<GraphObject> BindingFilter::operator[](const std::string&) {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


std::shared_ptr<GraphObject> BindingFilter::get(const std::string& var, const std::string& key) {
    auto search_var = var + "." + key;
    auto value = binding[search_var];
    if (value != nullptr) {
        return value;
    }
    auto search = cache.find(search_var);
    if (search != cache.end()) {
        return (*search).second;
    }
    else { // no esta en el cache ni el el binding original
        auto info = var_info[var];
        auto key_object_id = RelationalModel::get_id(key);

        auto var_value = binding[var];

        // TODO: add to cache
        // auto search_prop = SearchProperty(info.second, element_id.value, key_object_id);
        // cache.insert({ search_var, search_prop.get_value() });

        BPlusTree* bpt;
        ObjectId element_object_id;
        if (info.second == ObjectType::node) {
            Node node = static_cast<const Node&>(*var_value);
            element_object_id = node.id;
            bpt = RelationalModel::get_graph(info.first).prop2edge.get();
        }
        else {
            Edge edge = static_cast<const Edge&>(*var_value);
            element_object_id = edge.id;
            bpt = RelationalModel::get_graph(info.first).prop2edge.get();
        }
        auto it = bpt->get_range(
            Record(element_object_id, key_object_id, 0),
            Record(element_object_id, key_object_id, UINT64_MAX)
        );

        auto res = it->next();
        if (res != nullptr) {
            auto value_obj_id = ObjectId(res->ids[2]);
            return RelationalModel::get_object(value_obj_id);
        }
        else {
            return nullptr;
        }
    }
}
