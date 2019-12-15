#include "test_join.h"

#include "base/var/var_id.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/import/bulk_import.h"
#include "relational_model/physical_plan/binding_id.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"

using namespace std;

void TestJoin::get_all_node_properties_by_label(string label) {
    Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

    vector<VarId> vars {
		VarId(0),
		VarId(1),
		VarId(2)
	};

	vector<string> var_names {
		"first",
		"second",
		"third"
	};

    // s1: search all nodes with label
	GraphScan s1 = GraphScan(0, *graph.label2element, vars);
	auto input = make_shared<BindingId>(3);

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	int limit = 20;
	while (b != nullptr && count < limit) {
		b->print(var_names);
		b = root.next();
		count++;
	}
}