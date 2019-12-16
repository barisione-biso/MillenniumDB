#include "query_optimizer_label.h"

QueryOptimizerLabel::QueryOptimizerLabel
    (VarId element_var_id, VarId label_var_id, ElementType element_type)
    : element_var_id(element_var_id),
      label_var_id(label_var_id),
      element_type(element_type),
{ }

int QueryOptimizerLabel::get_heuristic() {
    if (element_var_id.is_term() && label_var_id.is_term()) {
        return -1;
    }

    if (element_var_id.is_term()) return 10; // Label(_,?)
    if (label_var_id.is_term())   return  5; // Label(?,_)
    else                          return  2; // Label(?,?)
}


void QueryOptimizerLabel::assign(VarId var_id) {

}


unique_ptr<GraphScan> QueryOptimizerLabel::get_scan() {
    return make_unique<GraphScan>(graph_id, bpt, terms, vars); //TODO: de donde sacar el graph_id y el bpt?
}