#include "query_optimizer_property.h"

QueryOptimizerProperty::QueryOptimizerProperty
    (VarId element_var_id, VarId key_var_id, VarId value_var_id, ElementType element_type)
    : element_var_id(element_var_id),
      key_var_id(key_var_id),
      value_var_id(value_var_id),
      element_type(element_type)
{ }

int QueryOptimizerProperty::get_heuristic() {
    // Property(_,_,?)         11
    // Property(?,_,_)         8
    // Property(?,_,?)         4
    // Property(?,?,?)         1
}


void QueryOptimizerProperty::assign(VarId var_id) {

}


unique_ptr<GraphScan> QueryOptimizerProperty::get_scan() {
    return make_unique<GraphScan>(graph_id, bpt, terms, vars); //TODO: de donde sacar el graph_id y el bpt?
}