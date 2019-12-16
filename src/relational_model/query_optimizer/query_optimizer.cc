#include "query_optimizer.h"

#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"

unique_ptr<BindingIdIter> QueryOptimizer::get_query_plan(std::vector<QueryOptimizerElement> elements) {
    Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);


    // caso borde: solo 1 elemento
    if (elements.size() == 1) {
        return elements[0].get_scan();
    }

    unique_ptr<BindingIdIter> current_root = nullptr;
    // else 2 or more scans are needed. Left nested loop join
    auto elements_size = elements.size();
    for (size_t i = 0; i < elements_size; i++) {
        int best_index = 0;
        int best_heuristic = elements[0].get_heuristic();

        for (size_t j = 0; j < elements_size; j++) {
            auto current_heuristic = elements[j].get_heuristic();
            if (current_heuristic > best_heuristic) {
                best_heuristic = current_heuristic;
                best_index = j;
            }
        }
        if (best_heuristic == -1) {
            throw std::logic_error("redundant scan"); // TODO: define what to do in this case (is this case possible?)
        }
        if (current_root == nullptr) {
            current_root = elements[best_heuristic].get_scan();
        }
        else {
            current_root = make_unique<IndexNestedLoopJoin>(std::move(current_root), elements[best_index].get_scan());
        }
    }

    vector<VarId> vars {
		VarId(0),
		VarId(1),
		VarId(2)
	};

	vector<ObjectId> terms { };
    return make_unique<GraphScan>(0, *graph.element2label, terms, vars);
}