#include "query_optimizer.h"

#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"


QueryOptimizer::QueryOptimizer() {}


unique_ptr<BindingIdIter> QueryOptimizer::get_query_plan(std::vector<QueryOptimizerElement*> elements) {

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
            throw std::logic_error("something is wrong with query optimizer"); // TODO: delete redundant check
        }
        auto assigned_vars = elements[best_index]->get_assigned();
        for (auto element : elements) {
            for (auto var : assigned_vars) {
                element->try_assign_var(var);
            }
        }
        elements[best_index]->assign();
        if (current_root == nullptr) {
            current_root = elements[best_index]->get_scan();
        }
        else {
            current_root = make_unique<IndexNestedLoopJoin>(std::move(current_root),
                            elements[best_index]->get_scan());
        }
    }

    return current_root;
}