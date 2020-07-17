#include "labeled_connection_plan.h"

#include <limits>

#include "relational_model/relational_model.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"
#include "storage/catalog/catalog.h"

using namespace std;

LabeledConnectionPlan::LabeledConnectionPlan(GraphId graph_id, ObjectId label_id, VarId node_from_var_id,
                                             VarId node_to_var_id, VarId edge_var_id) :
    graph_id(graph_id),
    label_id(label_id),
    node_from_var_id(node_from_var_id),
    node_to_var_id(node_to_var_id),
    edge_var_id(edge_var_id),
    node_from_assigned(false),
    node_to_assigned(false),
    edge_assigned(false) { }


LabeledConnectionPlan::LabeledConnectionPlan(const LabeledConnectionPlan& other) :
    graph_id(other.graph_id),
    label_id(other.label_id),
    node_from_var_id(other.node_from_var_id),
    node_to_var_id(other.node_to_var_id),
    edge_var_id(other.edge_var_id),
    node_from_assigned(other.node_from_assigned),
    node_to_assigned(other.node_to_assigned),
    edge_assigned(other.edge_assigned) { }


std::unique_ptr<JoinPlan> LabeledConnectionPlan::duplicate() {
    return make_unique<LabeledConnectionPlan>(*this);
}


void LabeledConnectionPlan::print(int indent, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "LabeledConnection(" << relational_model.get_graph_object(label_id)->to_string()
         << ", ?" << var_names[edge_var_id.id]
         << ", ?" << var_names[node_from_var_id.id]
         << ", ?" << var_names[node_to_var_id.id]
         << ")";
}


double LabeledConnectionPlan::estimate_cost() {
    return 100 + estimate_output_size();
}


double LabeledConnectionPlan::estimate_output_size() {
    if (edge_assigned) {
        return numeric_limits<double>::max();
    }
    auto labeled_connections = static_cast<double>(catalog.get_edge_count_for_label(graph_id, label_id));
    auto total_nodes         = static_cast<double>(catalog.get_node_count(graph_id));

    if (node_from_assigned) {
        if (node_to_assigned) {
            return labeled_connections / (total_nodes*total_nodes);
        } else {
            return labeled_connections / total_nodes;
        }
    } else {
        if (node_to_assigned) {
            return labeled_connections / total_nodes;
        } else {
            return labeled_connections;
        }
    }
}


void LabeledConnectionPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    for (auto& input_var : input_var_order) {
        if (node_to_var_id == input_var) {
            node_to_assigned = true;
        } else if (node_from_var_id == input_var) {
            node_from_assigned = true;
        } else if (edge_var_id == input_var) {
            edge_assigned = true;
        }
    }
}


// Must be consistent with the index scan returned in get_binding_id_iter()
vector<VarId> LabeledConnectionPlan::get_var_order() {
    vector<VarId> result;

    if (node_from_assigned || !node_to_assigned) { // LFTE
        result.push_back(node_from_var_id);
        result.push_back(node_to_var_id);
        result.push_back(edge_var_id);
    } else {                                       // LTFE
        result.push_back(node_to_var_id);
        result.push_back(node_from_var_id);
        result.push_back(edge_var_id);
    }
    return result;
}


/** LFTE | LTFE
 * ╔═╦═══════╦══════════╦════════╦══════════╦══════════╗
 * ║ ║ Label ║ NodeFrom ║ NodeTo ║   Edge   ║  index   ║
 * ╠═╬═══════╬══════════╬════════╬══════════╬══════════╣
 * ║1║  yes  ║     yes  ║   yes  ║    yes   ║     X    ║
 * ║2║  yes  ║     yes  ║   yes  ║    no    ║   LFTE   ║
 * ║3║  yes  ║     yes  ║   no   ║    yes   ║     X    ║
 * ║4║  yes  ║     yes  ║   no   ║    no    ║   LFTE   ║
 * ║5║  yes  ║     no   ║   yes  ║    yes   ║     X    ║
 * ║6║  yes  ║     no   ║   yes  ║    no    ║   LTFE   ║
 * ║7║  yes  ║     no   ║   no   ║    yes   ║     X    ║
 * ║8║  yes  ║     no   ║   no   ║    no    ║   LFTE   ║
 * ╚═╩═══════╩══════════╩════════╩══════════╩══════════╝
 */
unique_ptr<BindingIdIter> LabeledConnectionPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 4> ranges;
    if (node_from_assigned || !node_to_assigned) {
        ranges[0] = make_unique<Term>(label_id);
        ranges[1] = get_node_from_range();
        ranges[2] = get_node_to_range();
        ranges[3] = get_edge_range();
        return make_unique<IndexScan<4>>(relational_model.get_label_from_to_edge(), move(ranges));
    } else {
        ranges[0] = make_unique<Term>(label_id);
        ranges[1] = get_node_to_range();
        ranges[2] = get_node_from_range();
        ranges[3] = get_edge_range();
        return make_unique<IndexScan<4>>(relational_model.get_label_to_from_edge(), move(ranges));
    }
}


std::unique_ptr<ScanRange> LabeledConnectionPlan::get_node_from_range() {
    if (node_from_assigned) {
        return make_unique<AssignedVar>(node_from_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(node_from_var_id);
    } else {
        return make_unique<NamedGraphVar>(node_from_var_id, graph_id, ObjectType::node);
    }
}


std::unique_ptr<ScanRange> LabeledConnectionPlan::get_node_to_range() {
    if (node_to_assigned) {
        return make_unique<AssignedVar>(node_to_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(node_to_var_id);
    } else {
        return make_unique<NamedGraphVar>(node_to_var_id, graph_id, ObjectType::node);
    }
}


std::unique_ptr<ScanRange> LabeledConnectionPlan::get_edge_range() {
    if (edge_assigned) {
        // return make_unique<AssignedVar>(edge_var_id);
        throw std::logic_error("LabeledConnectionPlan: edge can't be assigned");
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(edge_var_id);
    } else {
        return make_unique<NamedGraphVar>(edge_var_id, graph_id, ObjectType::edge);
    }
}
