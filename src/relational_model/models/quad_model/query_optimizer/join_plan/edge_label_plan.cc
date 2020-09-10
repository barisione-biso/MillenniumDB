#include "edge_label_plan.h"

#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"

using namespace std;

EdgeLabelPlan::EdgeLabelPlan(GraphId graph_id, VarId edge_var_id, ObjectId label_id) :
    graph_id(graph_id),
    edge_var_id(edge_var_id),
    label_var_id(VarId::get_null()),
    label_id(label_id),
    edge_assigned(false),
    label_assigned( !label_id.is_null() ) { }


EdgeLabelPlan::EdgeLabelPlan(const EdgeLabelPlan& other) :
    graph_id(other.graph_id),
    edge_var_id(other.edge_var_id),
    label_var_id(other.label_var_id),
    label_id(other.label_id),
    edge_assigned(other.edge_assigned),
    label_assigned(other.label_assigned) { }


std::unique_ptr<JoinPlan> EdgeLabelPlan::duplicate() {
    return make_unique<EdgeLabelPlan>(*this);
}


void EdgeLabelPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "EdgeLabel(?" << var_names[edge_var_id.id];
    if (label_var_id.is_null()) {
        // TODO:
        // cout << ", " << relational_model.get_graph_object(label_id)->to_string();
    } else {
        cout << ", ?" << var_names[label_var_id.id];
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


double EdgeLabelPlan::estimate_cost() {
    return /*100.0 +*/ estimate_output_size();
}


double EdgeLabelPlan::estimate_output_size() {
    // TODO: remake
    return 1;
    // auto total_edges = static_cast<double>(catalog.get_edge_count(graph_id));
    // auto total_edge_labels = static_cast<double>(catalog.get_edge_labels(graph_id));
    // // edges with label `label_id`
    // auto edge_labels = static_cast<double>(catalog.get_edge_label_count(graph_id, label_id));

    // if (total_edges == 0) { // to avoid division by 0
    //     return 0;
    // }

    // if (label_assigned) {
    //     if (edge_assigned) {
    //         return edge_labels / total_edges;
    //     } else {
    //         return edge_labels;
    //     }
    // } else {
    //     if (edge_assigned) {
    //         return total_edge_labels / total_edges;
    //     } else {
    //         return total_edges;
    //     }
    // }
}


void EdgeLabelPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    for (auto& input_var : input_var_order) {
        if (edge_var_id == input_var) {
            edge_assigned = true;
        } else if (label_var_id == input_var) {
            label_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
vector<VarId> EdgeLabelPlan::get_var_order() {
    vector<VarId> result;
    if (edge_assigned || !label_assigned) {
        // cases 1, 2 and 4 uses EL
        result.push_back(edge_var_id);
        if (!label_var_id.is_null()) {
            result.push_back(label_var_id);
        }
    } else {
        // case 3 uses LE
        if (!label_var_id.is_null()) {
            result.push_back(label_var_id);
        }
        result.push_back(edge_var_id);
    }
    return result;
}


/**
 * ╔═╦══════════╦═════════╦═════════╗
 * ║ ║  EdgeId  ║ LabelId ║  Index  ║
 * ╠═╬══════════╬═════════╬═════════╣
 * ║1║    yes   ║   yes   ║    EL   ║ => any index works for this case
 * ║2║    yes   ║   no    ║    EL   ║
 * ║3║    no    ║   yes   ║    LN   ║
 * ║4║    no    ║   no    ║    EL   ║ => any index works for this case
 * ╚═╩══════════╩═════════╩═════════╝
 */
unique_ptr<BindingIdIter> EdgeLabelPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 2> ranges;
    // TODO: remake
    return nullptr;
    // if (edge_assigned || !label_assigned) {
    //     // cases 1, 2 and 4 uses EL
    //     ranges[0] = get_edge_range();
    //     ranges[1] = get_label_range();

    //     return make_unique<IndexScan<2>>(relational_model.get_edge2label(), move(ranges));
    // } else {
    //     // case 3 uses EL
    //     ranges[0] = get_label_range();
    //     ranges[1] = get_edge_range();

    //     return make_unique<IndexScan<2>>(relational_model.get_label2edge(), move(ranges));
    // }
}

std::unique_ptr<ScanRange> EdgeLabelPlan::get_edge_range() {
    if (edge_assigned) {
        return make_unique<AssignedVar>(edge_var_id);
    } else {
        return make_unique<DefaultGraphVar>(edge_var_id);
    }
}


std::unique_ptr<ScanRange> EdgeLabelPlan::get_label_range() {
    if (!label_id.is_null()) {
        return make_unique<Term>(label_id);
    } else if (label_assigned) {
        return make_unique<AssignedVar>(label_var_id);
    } else {
        return make_unique<DefaultGraphVar>(label_var_id);
    }
}
