#include "query_optimizer_property.h"

#include "relational_model/graph/relational_graph.h"

QueryOptimizerProperty::QueryOptimizerProperty(RelationalGraph& graph, VarId element_var_id, VarId key_var_id,
    VarId value_var_id, ElementType element_type, ObjectId key_object_id, ObjectId value_object_id) :
    graph(graph),
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


void QueryOptimizerProperty::assign() {
    assigned = true;
}


void QueryOptimizerProperty::try_assign_var(VarId var_id) {
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


std::vector<VarId> QueryOptimizerProperty::get_assigned() {
    vector<VarId> res;

    if (!element_assigned)
        res.push_back(element_var_id);

    if (!key_assigned)
        res.push_back(key_var_id);

    if (!value_assigned)
        res.push_back(value_var_id);

    return std::move(res);
}


unique_ptr<GraphScan> QueryOptimizerProperty::get_scan() {
    std::vector<ObjectId> terms;
    std::vector<VarId> vars;

    if (key_assigned) { // Property(_,?,_), Property(?,?,_), Property(_,?,?), Property(?,?,?)
        // term, var, var
        if (!key_object_id.is_null()) {
            terms.push_back(key_object_id);
            if (!value_object_id.is_null()) {
                terms.push_back(value_object_id);
            }
            else {
                vars.push_back(value_var_id);
            }
            vars.push_back(element_var_id);
        }
        return make_unique<GraphScan>(*graph.prop2element, terms, vars);
    }
    else { // Property(_,_,_), Property(?,_,_), Property(_,_,?), Property(?,_,?)
        vars.push_back(element_var_id);
        vars.push_back(key_var_id);
        vars.push_back(value_var_id);
        return make_unique<GraphScan>(*graph.element2prop, terms, vars);
    }
}