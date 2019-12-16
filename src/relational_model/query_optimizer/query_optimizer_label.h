#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_

#include <string>
#include <vector>

#include "base/var/var_id.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizerLabel : public QueryOptimizerElement {
private:
    VarId element_var_id;
    VarId label_var_id;
    ElementType element_type;

    std::vector<ObjectId> terms;
    std::vector<VarId> vars;

public:
    QueryOptimizerLabel(VarId element_var_id, VarId label_var_id, ElementType element_type);
    ~QueryOptimizerLabel() = default;
    int get_heuristic();
    void assign(VarId var_id);
    unique_ptr<GraphScan> get_scan();
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_
