#include "query_optimizer_lonely_node.h"

#include "relational_model/binding/binding_id.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "relational_model/physical_plan/binding_id_iter/node_enum.h"
#include "storage/catalog/catalog.h"

using namespace std;

QueryOptimizerLonelyNode::QueryOptimizerLonelyNode
    (GraphId graph_id, VarId element_var_id) :
    graph_id(graph_id),
    element_var_id(element_var_id)
{
    element_assigned = false;
}


int QueryOptimizerLonelyNode::get_heuristic() {
    if (assigned)              return -1;
    // else if (element_assigned) return 99;
    else                       return  0;
}


std::vector<VarId> QueryOptimizerLonelyNode::assign() {
    assigned = true;
    vector<VarId> res = { element_var_id };
    return res;
}


void QueryOptimizerLonelyNode::try_assign_var(VarId var_id) {
    if (assigned) {
        return;
    }
    if (element_var_id == var_id) {
        // TODO: para que esto se cumpla hay que eliminar redundancia
        // EJ:
        // SELECT *
        // MATCH (?n), (?n)
        throw std::logic_error("LonelyNode should appear once and only if no other node with "
                               "the same name is present in the Graph Pattern");
    }
}


unique_ptr<BindingIdIter> QueryOptimizerLonelyNode::get_scan() {
    return make_unique<NodeEnum>(graph_id, element_var_id);
}
