#include "query_optimizer_label.h"

#include "relational_model/binding/binding_id.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/term.h"

using namespace std;

QueryOptimizerLabel::QueryOptimizerLabel
    (GraphId graph_id, VarId element_var_id, VarId label_var_id, ObjectType element_type, ObjectId label_object_id) :
    graph_id(graph_id),
    element_var_id(element_var_id),
    label_var_id(label_var_id),
    element_type(element_type),
    label_object_id(label_object_id)
{
    element_assigned = false;
    label_assigned = !label_object_id.is_null();
}


int QueryOptimizerLabel::get_heuristic() {
    if (assigned) return -1;

    else if (element_assigned && label_assigned) return 99; // Label(_,_)
    else if (element_assigned)                   return 10; // Label(_,?)
    else if (label_assigned)                     return  5; // Label(?,_)
    else                                         return  2; // Label(?,?)
}


std::vector<VarId> QueryOptimizerLabel::assign() {
    assigned = true;
    vector<VarId> res;

    if (!element_assigned)
        res.push_back(element_var_id);

    if (!label_assigned)
        res.push_back(label_var_id);

    return res;
}


void QueryOptimizerLabel::try_assign_var(VarId var_id) {
    if (assigned) {
        return;
    }
    if (element_var_id == var_id) {
        element_assigned = true;
    }
    else if (label_var_id == var_id) {
        label_assigned = true;
    }
}

/**
 * ╔═╦════════════╦══════════╦═════════╗
 * ║ ║ Element_id ║ Label_id ║  index  ║
 * ╠═╬════════════╬══════════╬═════════╣
 * ║1║     yes    ║    yes   ║    EL   ║ => any index works for this case
 * ║2║     yes    ║    no    ║    EL   ║
 * ║3║     no     ║    yes   ║    LE   ║
 * ║4║     no     ║    no    ║    EL   ║ => any index works for this case
 * ╚═╩════════════╩══════════╩═════════╝
 */
unique_ptr<BindingIdIter> QueryOptimizerLabel::get_scan() {
    vector<unique_ptr<ScanRange>> ranges;
    if (element_assigned || !label_assigned) {
        // cases 1, 2 and 4 uses EL
        ranges.push_back(get_element_range());
        ranges.push_back(get_label_range());

        if (element_type == ObjectType::node) {
            return make_unique<GraphScan>(relational_model.get_node2label(), move(ranges));
        } else {
            return make_unique<GraphScan>(relational_model.get_edge2label(), move(ranges));
        }
    } else {
        // case 3
        ranges.push_back(get_label_range());
        ranges.push_back(get_element_range());

        if (element_type == ObjectType::node) {
            return make_unique<GraphScan>(relational_model.get_label2node(), move(ranges));
        } else {
            return make_unique<GraphScan>(relational_model.get_label2edge(), move(ranges));
        }
    }

}

std::unique_ptr<ScanRange> QueryOptimizerLabel::get_element_range() {
    if (element_assigned) {
        return make_unique<AssignedVar>(element_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(element_var_id);
    } else {
        return make_unique<NamedGraphVar>(element_var_id, graph_id);
    }
}


std::unique_ptr<ScanRange> QueryOptimizerLabel::get_label_range() {
    if (!label_object_id.is_null()) {
        return make_unique<Term>(label_object_id);
    } else if (label_assigned) {
        return make_unique<AssignedVar>(label_var_id);
    } else {
        return make_unique<DefaultGraphVar>(label_var_id);
    }
}
