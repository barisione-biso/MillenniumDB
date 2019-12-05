#include "tests/test_graph_import.h"

#include "base/var/var_id.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/import/bulk_import.h"
#include "relational_model/physical_plan/binding_id.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"

void TestGraphImport::bulk_import(std::string nodes_file, std::string edges_file) {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
	BulkImport import = BulkImport(nodes_file, edges_file, graph);
	import.start_import();
}

void TestGraphImport::test_label2element() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Id:"));
	var_names.insert(pair<int, string>(1, "Label:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1)); // Label:type1
	s1_vars.push_back(VarId(2)); // Node:?n

	GraphScan s1 = GraphScan(graph.graph_id, *graph.label2element, s1_vars);

	auto input = make_shared<BindingId>();
	ObjectId label_type_1 = graph.get_label_id(Label("grueling"));
	input->add(VarId(1), label_type_1);

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;
	int limit = 300;
	while (b != nullptr && ++count <= limit) {
		// b->print(var_names);
		auto n_id = b->search_id(VarId(2));
		Node node = graph.get_node(n_id->id);

		auto label_id = b->search_id(VarId(1));
		Label label = graph.get_label(label_id->id);

		cout << count << ") NodeId: " << node.get_id();
		cout << ":" << label.get_label_name() << "\t";
		cout << "\n";
		b = root.next();
	}
}

void TestGraphImport::test_element2label() {
	
}
