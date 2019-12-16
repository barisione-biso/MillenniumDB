#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_

#include <string>
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizerConnection : public QueryOptimizerElement {
public:
    int get_heuristic();
    void assign(VarId var_id);
    unique_ptr<GraphScan> get_scan();
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
