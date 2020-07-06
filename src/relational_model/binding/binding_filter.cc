#include "binding_filter.h"

#include "relational_model/relational_model.h"
#include "relational_model/graph/relational_graph.h"

using namespace std;

BindingFilter::BindingFilter(Binding& binding, map<string, GraphId>& var2graph_id,
                             set<string>& node_names, set<string>& edge_names) :
    binding(binding),
    var2graph_id(var2graph_id),
    node_names(node_names),
    edge_names(edge_names) { }


std::string BindingFilter::to_string() const {
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
        auto graph_id = var2graph_id[var];
        auto key_object_id = relational_model.get_string_id(key);
        auto var_value = binding[var];

        unique_ptr<BptIter<3>> it = nullptr;
        auto node_search = node_names.find(var);
        if (node_search != node_names.end()) {
            Node node = static_cast<const Node&>(*var_value);
            it = relational_model.get_node_key_value().get_range(
                RecordFactory::get(node.id, key_object_id, 0),
                RecordFactory::get(node.id, key_object_id, UINT64_MAX)
            );
        } else {
            Edge edge = static_cast<const Edge&>(*var_value);
            it = relational_model.get_edge_key_value().get_range(
                RecordFactory::get(edge.id, key_object_id, 0),
                RecordFactory::get(edge.id, key_object_id, UINT64_MAX)
            );
        }

        auto res = it->next();
        if (res != nullptr) {
            auto value_obj_id = ObjectId(res->ids[2]);
            auto res = relational_model.get_graph_object(value_obj_id);
            cache.insert({ search_var, res });
            return res;
        }
        else {
            cache.insert({ search_var, nullptr });
            return nullptr;
        }
    }
}
