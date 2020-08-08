#include "edge_property_plan.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"

#include "storage/catalog/catalog.h"

using namespace std;

EdgePropertyPlan::EdgePropertyPlan(GraphId graph_id, VarId edge_var_id,
                                   ObjectId key_id, VarId value_var_id) :
    graph_id       (graph_id),
    edge_var_id    (edge_var_id),
    key_var_id     (VarId::get_null()),
    value_var_id   (value_var_id),
    key_id         (key_id),
    value_id       (ObjectId::get_null()),
    edge_assigned  (false),
    key_assigned   (true),
    value_assigned (false) { }


EdgePropertyPlan::EdgePropertyPlan(GraphId graph_id, VarId edge_var_id,
                                   ObjectId key_id, ObjectId value_id) :
    graph_id       (graph_id),
    edge_var_id    (edge_var_id),
    key_var_id     (VarId::get_null()),
    value_var_id   (VarId::get_null()),
    key_id         (key_id),
    value_id       (value_id),
    edge_assigned  (false),
    key_assigned   (true),
    value_assigned (true) { }


EdgePropertyPlan::EdgePropertyPlan(const EdgePropertyPlan& other) :
    graph_id       (other.graph_id),
    edge_var_id    (other.edge_var_id),
    key_var_id     (other.key_var_id),
    value_var_id   (other.value_var_id),
    key_id         (other.key_id),
    value_id       (other.value_id),
    edge_assigned  (other.edge_assigned),
    key_assigned   (other.key_assigned),
    value_assigned (other.value_assigned) { }


std::unique_ptr<JoinPlan> EdgePropertyPlan::duplicate() {
    return make_unique<EdgePropertyPlan>(*this);
}


void EdgePropertyPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "EdgeProperty(?" << var_names[edge_var_id.id];
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

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}


double EdgePropertyPlan::estimate_cost() {
    return /*100.0 +*/ estimate_output_size();
}


double EdgePropertyPlan::estimate_output_size() {
    auto unique_values         = static_cast<double>(catalog.get_edge_key_unique_values(graph_id, key_id));
    auto edge_keys             = static_cast<double>(catalog.get_edge_key_count(graph_id, key_id));
    auto total_edges           = static_cast<double>(catalog.get_edge_count(graph_id));
    auto total_edge_properties = static_cast<double>(catalog.get_edge_properties(graph_id));

    if (total_edges == 0) { // To avoid division by 0
        return 0;
    }

    if (key_assigned) {
        if (unique_values == 0) { // To avoid division by 0
            return 0;
        }
        if (value_assigned) {
            if (edge_assigned) {
                return edge_keys / (unique_values * total_edges);
            } else {
                return edge_keys / unique_values;
            }
        } else {
            if (edge_assigned) {
                return edge_keys / total_edges;
            } else {
                return edge_keys;
            }
        }
    } else {
        if (value_assigned) {
            throw logic_error("fixed values without fixed key is not supported");
        } else {
            if (edge_assigned) {
                return total_edge_properties / total_edges;
            } else {
                return total_edge_properties;
            }
        }
    }
}


void EdgePropertyPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    for (auto& input_var : input_var_order) {
        if (edge_var_id == input_var) {
            edge_assigned = true;
        } else if (key_var_id == input_var) {
            key_assigned = true;
        } else if (value_var_id == input_var) {
            value_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
vector<VarId> EdgePropertyPlan::get_var_order() {
    vector<VarId> result;
    if (edge_assigned) {
        // NKV
        result.push_back(edge_var_id);
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
                result.push_back(edge_var_id);
            } else {              // Case 6: JUST KEY
                if (graph_id.is_default()) {
                    // KVN
                    if (!key_var_id.is_null()) {
                        result.push_back(key_var_id);
                    }
                    if (!value_var_id.is_null()) {
                        result.push_back(value_var_id);
                    }
                    result.push_back(edge_var_id);
                } else {
                    // KNV
                    if (!key_var_id.is_null()) {
                        result.push_back(key_var_id);
                    }
                    result.push_back(edge_var_id);
                    if (!value_var_id.is_null()) {
                        result.push_back(value_var_id);
                    }
                }
            }
        } else {
            // NKV
            result.push_back(edge_var_id);
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
 * ║ ║ EdgeId ║ KeyId ║ ValueId ║ default_graph ║ named_graph ║
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
unique_ptr<BindingIdIter> EdgePropertyPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 3> ranges;

    if (edge_assigned) {
        if (!key_assigned && value_assigned) { // Case 3: EDGE AND VALUE
            throw logic_error("fixed values with open key is not supported");
        }
        // cases 1,2 and 4 uses NKV, and case 3 throws exception
        ranges[0] = get_edge_range();
        ranges[1] = get_key_range();
        ranges[2] = get_value_range();

        return make_unique<IndexScan<3>>(relational_model.get_edge_key_value(), move(ranges));
    } else {
        if (key_assigned) {
            if (value_assigned) { // Case 5: KEY AND VALUE
                ranges[0] = get_key_range();
                ranges[1] = get_value_range();
                ranges[2] = get_edge_range();

                return make_unique<IndexScan<3>>(relational_model.get_key_value_edge(), move(ranges));
            } else {              // Case 6: JUST KEY
                if (graph_id.is_default()) {
                    ranges[0] = get_key_range();
                    ranges[1] = get_value_range();
                    ranges[2] = get_edge_range();

                    return make_unique<IndexScan<3>>(relational_model.get_key_value_edge(), move(ranges));
                } else {
                    ranges[0] = get_key_range();
                    ranges[1] = get_edge_range();
                    ranges[2] = get_value_range();

                    return make_unique<IndexScan<3>>(relational_model.get_key_edge_value(), move(ranges));
                }
            }
        } else {
            if (value_assigned) { // Case 7: JUST VALUE
                throw logic_error("fixed values with open key is not supported");
            } else {              // Case 8: NOTHING
                ranges[0] = get_edge_range();
                ranges[1] = get_key_range();
                ranges[2] = get_value_range();

                return make_unique<IndexScan<3>>(relational_model.get_edge_key_value(), move(ranges));
            }
        }
    }
}


std::unique_ptr<ScanRange> EdgePropertyPlan::get_edge_range() {
    if (edge_assigned) {
        return make_unique<AssignedVar>(edge_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(edge_var_id);
    } else {
        return make_unique<NamedGraphVar>(edge_var_id, graph_id, ObjectType::edge);
    }
}


std::unique_ptr<ScanRange> EdgePropertyPlan::get_key_range() {
    if (!key_id.is_null()) {
        return make_unique<Term>(key_id);
    } else if (key_assigned) {
        return make_unique<AssignedVar>(key_var_id);
    } else {
        return make_unique<DefaultGraphVar>(key_var_id);
    }
}


std::unique_ptr<ScanRange> EdgePropertyPlan::get_value_range() {
    if (!value_id.is_null()) {
        return make_unique<Term>(value_id);
    } else if (value_assigned) {
        return make_unique<AssignedVar>(value_var_id);
    } else {
        return make_unique<DefaultGraphVar>(value_var_id);
    }
}
