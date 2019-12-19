#include "query_optimizer_label.h"

#include "relational_model/graph/relational_graph.h"

QueryOptimizerLabel::QueryOptimizerLabel
    (RelationalGraph& graph, VarId element_var_id, VarId label_var_id, ElementType element_type, ObjectId label_object_id) :
    graph(graph),
    element_var_id(element_var_id),
    label_var_id(label_var_id),
    element_type(element_type),
    label_object_id(label_object_id)
{
    element_assigned = false;
    label_assigned = !label_object_id.is_null();
}


void QueryOptimizerLabel::assign() {
    assigned = true;
}


int QueryOptimizerLabel::get_heuristic() {
    if (assigned) return -1;

    else if (element_assigned && label_assigned) return 99; // Label(_,_)
    else if (element_assigned)                   return 10; // Label(_,?)
    else if (label_assigned)                     return  5; // Label(?,_)
    else                                         return  2; // Label(?,?)
}


std::vector<VarId> QueryOptimizerLabel::get_assigned() {
    vector<VarId> res;

    if (!element_assigned)
        res.push_back(element_var_id);

    if (!label_assigned)
        res.push_back(label_var_id);

    return std::move(res);
}


void QueryOptimizerLabel::try_assign_var(VarId var_id) {
    if (element_var_id == var_id) {
        element_assigned = true;
    }
    else if (label_var_id == var_id) {
        label_assigned = true;
    }
}


unique_ptr<GraphScan> QueryOptimizerLabel::get_scan() {
    vector<pair<ObjectId, int>> terms;
    vector<pair<VarId, int>> vars;

    if (label_assigned) { // Label(?,_) or Label(_,_)
        if (label_object_id.is_null()) {
            vars.push_back(make_pair(label_var_id, 0));
        }
        else {
            terms.push_back(make_pair(label_object_id, 0));
        }
        vars.push_back(make_pair(element_var_id, 1));
        return make_unique<GraphScan>(*graph.label2element, std::move(terms), std::move(vars));
    }
    else { // Label(?,?) or Label(_,?)
        vars.push_back(make_pair(element_var_id, 0));
        vars.push_back(make_pair(label_var_id, 1));
        return make_unique<GraphScan>(*graph.element2label, std::move(terms), std::move(vars));
    }
}