#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_PROPERTY_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_PROPERTY_H_

#include <string>
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizerProperty : public QueryOptimizerElement {
private:
    VarId element_var_id;
    VarId key_var_id;
    VarId value_var_id;
    ElementType element_type;

    std::vector<ObjectId> terms;
    std::vector<VarId> vars;

public:
    QueryOptimizerProperty(VarId element_var_id, VarId key_var_id, VarId value_var_id, ElementType element_type);
    ~QueryOptimizerProperty() = default;
    int get_heuristic();
    void assign(VarId var_id);
    unique_ptr<GraphScan> get_scan();
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_PROPERTY_H_
