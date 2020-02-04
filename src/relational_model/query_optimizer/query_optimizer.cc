#include "query_optimizer.h"

#include "base/binding/binding_iter.h"
#include "base/parser/logical_plan/op/op.h"
#include "relational_model/binding/binding_id.h"
#include "relational_model/binding/binding_id_iter.h"
#include "relational_model/physical_plan/physical_plan_generator.h"
#include "relational_model/physical_plan/binding_id_iter/index_nested_loop_join.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

using namespace std;

QueryOptimizer::QueryOptimizer(RelationalGraph& graph, ObjectFile& obj_file)
    : graph(graph), obj_file(obj_file) { }


unique_ptr<BindingIter> QueryOptimizer::get_select_plan(unique_ptr<OpSelect> op_select) {
    PhysicalPlanGenerator generator = PhysicalPlanGenerator(graph, obj_file);
    return generator.exec(*op_select);
}
