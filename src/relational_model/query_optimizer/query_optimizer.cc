#include "query_optimizer.h"

#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/physical_plan/operators/graph_scan.h"
#include "relational_model/physical_plan/operators/projection.h"
#include "relational_model/physical_plan/operators/index_nested_loop_join.h"

using namespace std;

QueryOptimizer::QueryOptimizer(ObjectFile& obj_file, int_fast32_t global_vars)
    : obj_file(obj_file), global_vars(global_vars)
{}


unique_ptr<BindingIter> QueryOptimizer::get_query_plan(vector<QueryOptimizerElement*> elements,
    map<string, string> constants, vector<string> names, vector<int_fast32_t> var_positions)
{
    auto join_plan = get_join_plan(elements);
    return make_unique<Projection>(obj_file, move(join_plan), move(constants), move(names),
        move(var_positions), global_vars);
}


unique_ptr<BindingIdIter> QueryOptimizer::get_join_plan(vector<QueryOptimizerElement*> elements) {

    unique_ptr<BindingIdIter> current_root = nullptr;

    auto elements_size = elements.size();
    for (size_t i = 0; i < elements_size; i++) {
        int best_index = 0;
        int best_heuristic = elements[0]->get_heuristic();

        for (size_t j = 1; j < elements_size; j++) {
            auto current_heuristic = elements[j]->get_heuristic();
            if (current_heuristic > best_heuristic) {
                best_heuristic = current_heuristic;
                best_index = j;
            }
        }
        if (best_heuristic == -1) {
            throw logic_error("something is wrong with query optimizer"); // TODO: delete redundant check
        }
        elements[best_index]->assign(); // TODO: que assign devuelva la lista de asignados
        auto assigned_vars = elements[best_index]->get_assigned();

        for (auto element : elements) {
            for (auto var : assigned_vars) {
                element->try_assign_var(var);
            }
        }
        if (current_root == nullptr) {
            current_root = elements[best_index]->get_scan();
        }
        else {
            current_root = make_unique<IndexNestedLoopJoin>(move(current_root), elements[best_index]->get_scan());
        }
    }

    return current_root;
}