#include "query_optimizer_property.h"

#include "relational_model/binding/binding_id.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/term.h"

using namespace std;

QueryOptimizerProperty::QueryOptimizerProperty(GraphId graph_id, VarId element_var_id, VarId key_var_id,
    VarId value_var_id, ObjectType element_type, ObjectId key_object_id, ObjectId value_object_id) :
    graph_id(graph_id),
    element_var_id(element_var_id),
    key_var_id(key_var_id),
    value_var_id(value_var_id),
    element_type(element_type),
    key_object_id(key_object_id),
    value_object_id(value_object_id)
{
    element_assigned = false;
    key_assigned = !key_object_id.is_null();
    value_assigned = !value_object_id.is_null();
}


int QueryOptimizerProperty::get_heuristic() {
    if (assigned) return -1;

    else if (element_assigned && key_assigned && value_assigned) return 99; // Property(_,_,_)
    else if (element_assigned && key_assigned)                   return 11; // Property(_,_,?)
    else if (key_assigned && value_assigned)                     return  8; // Property(?,_,_)
    else if (element_assigned && key_assigned)                   return  4; // Property(?,_,?)
    else                                                         return  1; // Property(?,?,?)
}


void QueryOptimizerProperty::try_assign_var(VarId var_id) {
    if (assigned) {
        return;
    }
    if (element_var_id == var_id) {
        element_assigned = true;
    }
    else if (key_var_id == var_id) {
        key_assigned = true;
    }
    else if (value_var_id == var_id) {
        value_assigned = true;
    }
}


std::vector<VarId> QueryOptimizerProperty::assign() {
    assigned = true;
    vector<VarId> res;

    if (!element_assigned)
        res.push_back(element_var_id);

    if (!key_assigned)
        res.push_back(key_var_id);

    if (!value_assigned)
        res.push_back(value_var_id);

    return res;
}

/**
 * ╔═╦════════════╦════════╦══════════╦═══════════════╦═════════════╗
 * ║ ║ Element_id ║ Key_id ║ Value_id ║ default_graph ║ named_graph ║
 * ╠═╬════════════╬════════╬══════════╬═══════════════╬═════════════╣
 * ║1║     yes    ║   yes  ║    yes   ║      EKV      ║     EKV     ║ => any index works for this case
 * ║2║     yes    ║   yes  ║    no    ║      EKV      ║     EKV     ║ => KEV also works
 * ║3║     yes    ║   no   ║    yes   ║     ERROR     ║    ERROR    ║
 * ║4║     yes    ║   no   ║    no    ║      EKV      ║     EKV     ║
 * ║5║     no     ║   yes  ║    yes   ║      KVE      ║     KVE     ║
 * ║6║     no     ║   yes  ║    no    ║      KVE      ║     KEV     ║ => only case index depends on graph
 * ║7║     no     ║   no   ║    yes   ║     ERROR     ║    ERROR    ║
 * ║8║     no     ║   no   ║    no    ║      EKV      ║     EKV     ║ => any index works for this case
 * ╚═╩════════════╩════════╩══════════╩═══════════════╩═════════════╝
 */
unique_ptr<BindingIdIter> QueryOptimizerProperty::get_scan() {
    vector<unique_ptr<ScanRange>> ranges;

    if (element_assigned) {
        if (!key_assigned && value_assigned) { // Case 3: ELEMENT AND VALUE
            throw logic_error("fixed values with open key is not supported");
        }
        // cases 1,2 and 4 uses EKV, and case 3 throws exception
        ranges.push_back(get_element_range());
        ranges.push_back(get_key_range());
        ranges.push_back(get_value_range());

        if (element_type == ObjectType::node) {
            return make_unique<GraphScan>(relational_model.get_node_key_value(), move(ranges));
        } else {
            return make_unique<GraphScan>(relational_model.get_edge_key_value(), move(ranges));
        }
    } else {
        if (key_assigned) {
            if (value_assigned) { // Case 5: KEY AND VALUE
                ranges.push_back(get_key_range());
                ranges.push_back(get_value_range());
                ranges.push_back(get_element_range());

                if (element_type == ObjectType::node) {
                    return make_unique<GraphScan>(relational_model.get_key_value_node(), move(ranges));
                } else {
                    return make_unique<GraphScan>(relational_model.get_key_value_edge(), move(ranges));
                }
            } else {              // Case 6: JUST KEY
                if (graph_id.is_default()) {
                    ranges.push_back(get_key_range());
                    ranges.push_back(get_value_range());
                    ranges.push_back(get_element_range());

                    if (element_type == ObjectType::node) {
                        return make_unique<GraphScan>(relational_model.get_key_value_node(), move(ranges));
                    } else {
                        return make_unique<GraphScan>(relational_model.get_key_value_edge(), move(ranges));
                    }
                } else {
                    ranges.push_back(get_key_range());
                    ranges.push_back(get_element_range());
                    ranges.push_back(get_value_range());

                    if (element_type == ObjectType::node) {
                        return make_unique<GraphScan>(relational_model.get_key_node_value(), move(ranges));
                    } else {
                        return make_unique<GraphScan>(relational_model.get_key_edge_value(), move(ranges));
                    }
                }
            }
        } else {
            if (value_assigned) { // Case 7: JUST VALUE
                throw logic_error("fixed values with open key is not supported");
            } else {              // Case 8: NOTHING
                ranges.push_back(get_element_range());
                ranges.push_back(get_key_range());
                ranges.push_back(get_value_range());

                if (element_type == ObjectType::node) {
                    return make_unique<GraphScan>(relational_model.get_node_key_value(), move(ranges));
                } else {
                    return make_unique<GraphScan>(relational_model.get_edge_key_value(), move(ranges));
                }
            }
        }
    }
}

std::unique_ptr<ScanRange> QueryOptimizerProperty::get_element_range() {
    if (element_assigned) {
        return make_unique<AssignedVar>(element_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(element_var_id);
    } else {
        return make_unique<NamedGraphVar>(element_var_id, graph_id);
    }
}


std::unique_ptr<ScanRange> QueryOptimizerProperty::get_key_range() {
    if (!key_object_id.is_null()) {
        return make_unique<Term>(key_object_id);
    } else if (key_assigned) {
        return make_unique<AssignedVar>(key_var_id);
    } else {
        return make_unique<DefaultGraphVar>(key_var_id);
    }
}


std::unique_ptr<ScanRange> QueryOptimizerProperty::get_value_range() {
    if (!value_object_id.is_null()) {
        return make_unique<Term>(value_object_id);
    } else if (value_assigned) {
        return make_unique<AssignedVar>(value_var_id);
    } else {
        return make_unique<DefaultGraphVar>(value_var_id);
    }
}
