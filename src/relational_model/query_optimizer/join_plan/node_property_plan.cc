#include "node_property_plan.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"

#include "storage/catalog/catalog.h"

using namespace std;

NodePropertyPlan::NodePropertyPlan(GraphId graph_id, VarId node_var_id,
                                   ObjectId key_id, VarId value_var_id) :
    graph_id       (graph_id),
    node_var_id    (node_var_id),
    key_var_id     (VarId::get_null()),
    value_var_id   (value_var_id),
    key_id         (key_id),
    value_id       (ObjectId::get_null()),
    node_assigned  (false),
    key_assigned   (true),
    value_assigned (false) { }


NodePropertyPlan::NodePropertyPlan(GraphId graph_id, VarId node_var_id,
                                   ObjectId key_id, ObjectId value_id) :
    graph_id       (graph_id),
    node_var_id    (node_var_id),
    key_var_id     (VarId::get_null()),
    value_var_id   (VarId::get_null()),
    key_id         (key_id),
    value_id       (value_id),
    node_assigned  (false),
    key_assigned   (true),
    value_assigned (true) { }


NodePropertyPlan::NodePropertyPlan(const NodePropertyPlan& other) :
    graph_id(other.graph_id),
    node_var_id(other.node_var_id),
    key_var_id(other.key_var_id),
    value_var_id(other.value_var_id),
    key_id(other.key_id),
    value_id(other.value_id),
    node_assigned(other.node_assigned),
    key_assigned(other.key_assigned),
    value_assigned(other.value_assigned) { }


std::unique_ptr<JoinPlan> NodePropertyPlan::duplicate() {
    return make_unique<NodePropertyPlan>(*this);
}


void NodePropertyPlan::print(int indent, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "NodeProperty(?" << var_names[node_var_id.id];
    if (key_var_id.is_null()) {
        cout << ", " << relational_model.get_graph_object(key_id)->to_string();
    } else {
        cout << ", ?" << var_names[key_var_id.id];
    }

    if (value_var_id.is_null()) {
        cout << ", " << relational_model.get_graph_object(value_id)->to_string();
    } else {
        cout << ", ?" << var_names[value_var_id.id];
    }
    cout << ")";
}


double NodePropertyPlan::estimate_cost() {
    return 100 + estimate_output_size();
}


double NodePropertyPlan::estimate_output_size() {
    auto unique_values         = static_cast<double>(catalog.get_node_count_for_key(graph_id, key_id)); //TODO:
    auto node_keys             = static_cast<double>(catalog.get_node_count_for_key(graph_id, key_id));
    auto total_nodes           = static_cast<double>(catalog.get_node_count(graph_id));
    auto total_node_properties = static_cast<double>(catalog.get_node_properties(graph_id));

    if (key_assigned) {
        if (value_assigned) {
            if (node_assigned) {
                return node_keys / (unique_values * total_nodes);
            } else {
                return node_keys / unique_values;
            }
        } else {
            if (node_assigned) {
                return node_keys / total_nodes;
            } else {
                return node_keys;
            }
        }
    } else {
        if (value_assigned) {
            throw logic_error("fixed values without fixed key is not supported");
        } else {
            if (node_assigned) {
                return total_node_properties / total_nodes;
            } else {
                return total_node_properties;
            }
        }
    }
}


void NodePropertyPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    for (auto& input_var : input_var_order) {
        if (node_var_id == input_var) {
            node_assigned = true;
        } else if (key_var_id == input_var) {
            key_assigned = true;
        } else if (value_var_id == input_var) {
            value_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
vector<VarId> NodePropertyPlan::get_var_order() {
    vector<VarId> result;
    if (node_assigned) {
        // NKV
        result.push_back(node_var_id);
        if (!key_var_id.is_null()) {
            result.push_back(key_var_id);
        }
        if (!value_var_id.is_null()) {
            result.push_back(value_var_id);
        }
    } else {
        if (key_assigned) {
            if (value_assigned) { // Case 5: KEY AND VALUE
                // KVN
                if (!key_var_id.is_null()) {
                    result.push_back(key_var_id);
                }
                if (!value_var_id.is_null()) {
                    result.push_back(value_var_id);
                }
                result.push_back(node_var_id);
            } else {              // Case 6: JUST KEY
                if (graph_id.is_default()) {
                    // KVN
                    if (!key_var_id.is_null()) {
                        result.push_back(key_var_id);
                    }
                    if (!value_var_id.is_null()) {
                        result.push_back(value_var_id);
                    }
                    result.push_back(node_var_id);
                } else {
                    // KNV
                    if (!key_var_id.is_null()) {
                        result.push_back(key_var_id);
                    }
                    result.push_back(node_var_id);
                    if (!value_var_id.is_null()) {
                        result.push_back(value_var_id);
                    }
                }
            }
        } else {
            // NKV
            result.push_back(node_var_id);
            if (!key_var_id.is_null()) {
                result.push_back(key_var_id);
            }
            if (!value_var_id.is_null()) {
                result.push_back(value_var_id);
            }
        }
    }
    return result;
}


/**
 * ╔═╦════════╦═══════╦═════════╦═══════════════╦═════════════╗
 * ║ ║ NodeId ║ KeyId ║ ValueId ║ default_graph ║ named_graph ║
 * ╠═╬════════╬═══════╬═════════╬═══════════════╬═════════════╣
 * ║1║   yes  ║  yes  ║   yes   ║      NKV      ║     NKV     ║ => any index works for this case
 * ║2║   yes  ║  yes  ║   no    ║      NKV      ║     NKV     ║ => KEV also works
 * ║3║   yes  ║  no   ║   yes   ║     ERROR     ║    ERROR    ║
 * ║4║   yes  ║  no   ║   no    ║      NKV      ║     NKV     ║
 * ║5║   no   ║  yes  ║   yes   ║      KVN      ║     KVN     ║
 * ║6║   no   ║  yes  ║   no    ║      KVN      ║     KNV     ║ => only case index depends on graph
 * ║7║   no   ║  no   ║   yes   ║     ERROR     ║    ERROR    ║
 * ║8║   no   ║  no   ║   no    ║      NKV      ║     NKV     ║ => any index works for this case
 * ╚═╩════════╩═══════╩═════════╩═══════════════╩═════════════╝
 */
unique_ptr<BindingIdIter> NodePropertyPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 3> ranges;

    if (node_assigned) {
        if (!key_assigned && value_assigned) { // Case 3: NODE AND VALUE
            throw logic_error("fixed values with open key is not supported");
        }
        // cases 1,2 and 4 uses NKV, and case 3 throws exception
        ranges[0] = get_node_range();
        ranges[1] = get_key_range();
        ranges[2] = get_value_range();

        return make_unique<IndexScan<3>>(relational_model.get_node_key_value(), move(ranges));
    } else {
        if (key_assigned) {
            if (value_assigned) { // Case 5: KEY AND VALUE
                ranges[0] = get_key_range();
                ranges[1] = get_value_range();
                ranges[2] = get_node_range();

                return make_unique<IndexScan<3>>(relational_model.get_key_value_node(), move(ranges));
            } else {              // Case 6: JUST KEY
                if (graph_id.is_default()) {
                    ranges[0] = get_key_range();
                    ranges[1] = get_value_range();
                    ranges[2] = get_node_range();

                    return make_unique<IndexScan<3>>(relational_model.get_key_value_node(), move(ranges));
                } else {
                    ranges[0] = get_key_range();
                    ranges[1] = get_node_range();
                    ranges[2] = get_value_range();

                    return make_unique<IndexScan<3>>(relational_model.get_key_node_value(), move(ranges));
                }
            }
        } else {
            if (value_assigned) { // Case 7: JUST VALUE
                throw logic_error("fixed values with open key is not supported");
            } else {              // Case 8: NOTHING
                ranges[0] = get_node_range();
                ranges[1] = get_key_range();
                ranges[2] = get_value_range();

                return make_unique<IndexScan<3>>(relational_model.get_node_key_value(), move(ranges));
            }
        }
    }
}


std::unique_ptr<ScanRange> NodePropertyPlan::get_node_range() {
    if (node_assigned) {
        return make_unique<AssignedVar>(node_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(node_var_id);
    } else {
        return make_unique<NamedGraphVar>(node_var_id, graph_id, ObjectType::node);
    }
}


std::unique_ptr<ScanRange> NodePropertyPlan::get_key_range() {
    if (!key_id.is_null()) {
        return make_unique<Term>(key_id);
    } else if (key_assigned) {
        return make_unique<AssignedVar>(key_var_id);
    } else {
        return make_unique<DefaultGraphVar>(key_var_id);
    }
}


std::unique_ptr<ScanRange> NodePropertyPlan::get_value_range() {
    if (!value_id.is_null()) {
        return make_unique<Term>(value_id);
    } else if (value_assigned) {
        return make_unique<AssignedVar>(value_var_id);
    } else {
        return make_unique<DefaultGraphVar>(value_var_id);
    }
}
