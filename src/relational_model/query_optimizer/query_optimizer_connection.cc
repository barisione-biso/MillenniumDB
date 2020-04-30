#include "query_optimizer_connection.h"

#include "relational_model/binding/binding_id.h"
#include "relational_model/relational_model.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/named_graph_var.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/term.h"

using namespace std;

QueryOptimizerConnection::QueryOptimizerConnection(GraphId graph_id, VarId node_from_var_id,
                                                   VarId node_to_var_id, VarId edge_var_id)
    : graph_id(graph_id),
        node_from_var_id(node_from_var_id),
        node_to_var_id(node_to_var_id),
        edge_var_id(edge_var_id)
{
    node_from_assigned = false;
    node_to_assigned = false;
    edge_assigned = false;
}


int QueryOptimizerConnection::get_heuristic() {
    if (assigned) return -1;

    else if (node_from_assigned && node_to_assigned && edge_assigned) return 99; // Connection(_,_,_)
    else if (node_from_assigned && node_to_assigned)                  return  9; // Connection(_,?,_)
    else if (node_from_assigned)                                      return  7; // Connection(_,?,?)
    else if (node_to_assigned)                                        return  6; // Connection(?,?,_)
    else                                                              return  1; // Connection(?,?,?)
}


void QueryOptimizerConnection::try_assign_var(VarId var_id) {
    if (assigned) {
        return;
    }
    if (node_from_var_id == var_id) {
        node_from_assigned = true;
    }
    if (node_to_var_id == var_id) { // not else if because from_var_id may be equal to to_var_id
        node_to_assigned = true;
    }
    else if (edge_var_id == var_id) {
        edge_assigned = true;
    }
}


std::vector<VarId> QueryOptimizerConnection::assign() {
    assigned = true;

    vector<VarId> res;

    if (!node_from_assigned)
        res.push_back(node_from_var_id);

    if (!node_to_assigned)
        res.push_back(node_to_var_id);

    if (!edge_assigned)
        res.push_back(edge_var_id);

    return res;
}


/** FTE | TEF | EFT
 * ╔═╦══════════╦════════╦══════════╦══════════╗
 * ║ ║ NodeFrom ║  Edge  ║  NodeTo  ║  index   ║
 * ╠═╬══════════╬════════╬══════════╬══════════╣
 * ║1║     yes  ║   yes  ║    yes   ║    EFT   ║ => any index works for this case
 * ║2║     yes  ║   yes  ║    no    ║    EFT   ║
 * ║3║     yes  ║   no   ║    yes   ║    FTE   ║
 * ║4║     yes  ║   no   ║    no    ║    FTE   ║
 * ║5║     no   ║   yes  ║    yes   ║    TEF   ║
 * ║6║     no   ║   yes  ║    no    ║    EFT   ║
 * ║7║     no   ║   no   ║    yes   ║    TEF   ║
 * ║8║     no   ║   no   ║    no    ║    EFT   ║ => any index works for this case
 * ╚═╩══════════╩════════╩══════════╩══════════╝
 */
unique_ptr<BindingIdIter> QueryOptimizerConnection::get_scan() {
    vector<unique_ptr<ScanRange>> ranges;

    if (node_from_assigned) {
        if (edge_assigned) { // CASES 1 and 2 => EFT
            ranges.push_back(get_edge_range());
            ranges.push_back(get_node_from_range());
            ranges.push_back(get_node_to_range());

            return make_unique<GraphScan>(relational_model.get_edge_from_to(), move(ranges));
        } else { // CASES 3 and 4 => FTE
            ranges.push_back(get_node_from_range());
            ranges.push_back(get_node_to_range());
            ranges.push_back(get_edge_range());

            return make_unique<GraphScan>(relational_model.get_from_to_edge(), move(ranges));
        }
    } else {
        if (node_to_assigned) { // CASES 5 and 7 => TEF
            ranges.push_back(get_node_to_range());
            ranges.push_back(get_edge_range());
            ranges.push_back(get_node_from_range());

            return make_unique<GraphScan>(relational_model.get_to_edge_from(), move(ranges));
        } else { // CASES 6 and 8 => EFT
            ranges.push_back(get_edge_range());
            ranges.push_back(get_node_from_range());
            ranges.push_back(get_node_to_range());

            return make_unique<GraphScan>(relational_model.get_edge_from_to(), move(ranges));
        }
    }
}


std::unique_ptr<ScanRange> QueryOptimizerConnection::get_node_from_range() {
    if (node_from_assigned) {
        return make_unique<AssignedVar>(node_from_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(node_from_var_id);
    } else {
        return make_unique<NamedGraphVar>(node_from_var_id, graph_id);
    }
}


std::unique_ptr<ScanRange> QueryOptimizerConnection::get_node_to_range() {
    if (node_to_assigned) {
        return make_unique<AssignedVar>(node_to_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(node_to_var_id);
    } else {
        return make_unique<NamedGraphVar>(node_to_var_id, graph_id);
    }
}


std::unique_ptr<ScanRange> QueryOptimizerConnection::get_edge_range() {
    if (edge_assigned) {
        return make_unique<AssignedVar>(edge_var_id);
    } else if (graph_id.is_default()) {
        return make_unique<DefaultGraphVar>(edge_var_id);
    } else {
        return make_unique<NamedGraphVar>(edge_var_id, graph_id);
    }
}
