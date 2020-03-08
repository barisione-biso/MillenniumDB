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
        // TODO: esta busqueda se debería hacer solo 1 vez por consulta
        auto key_object_id = RelationalModel::get_string_unmasked_id(key);
        if (key_object_id.is_null()) {
            cout << "key_object_id null\n";
            return nullptr;
        }
        if (key_object_id.not_found()) {
            cout << "key_object_id not found\n";
            return nullptr;
        }
        auto var_value = binding[var];

        // TODO: add to cache
        // auto search_prop = SearchProperty(info.second, element_id.value, key_object_id);
        // cache.insert({ search_var, search_prop.get_value() });

        BPlusTree* bpt;
        ObjectId element_object_id;
        uint64_t mask;
        if (info.second == ObjectType::node) {
            Node node = static_cast<const Node&>(*var_value);
            element_object_id = node.id;
            bpt = RelationalModel::get_graph(info.first).node2prop.get();
            mask = NODE_MASK;
        }
        else {
            Edge edge = static_cast<const Edge&>(*var_value);
            element_object_id = edge.id;
            bpt = RelationalModel::get_graph(info.first).edge2prop.get();
            mask = EDGE_MASK;
        }
        auto it = bpt->get_range(
            Record(element_object_id | mask, key_object_id, 0),
            Record(element_object_id | mask, key_object_id, UINT64_MAX)
        );

        auto res = it->next();
        if (res != nullptr) {
            auto value_obj_id = ObjectId(res->ids[2]);
            cout << "Found obj_id " << value_obj_id << endl;
            return RelationalModel::get_graph_object(value_obj_id);
        }
        else {
            return nullptr;
        }
    }
}
