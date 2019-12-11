#include "tests/test_graph_import.h"

#include "base/var/var_id.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/import/bulk_import.h"
#include "relational_model/physical_plan/binding_id.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"

#include <chrono>

void TestGraphImport::bulk_import(std::string nodes_file, std::string edges_file) {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
	BulkImport import = BulkImport(nodes_file, edges_file, graph);
	import.start_import();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "Bulk Import duration: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_label2element() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Id:"));
	var_names.insert(pair<int, string>(2, "Label:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(2));
	s1_vars.push_back(VarId(1));

	GraphScan s1 = GraphScan(graph.graph_id, *graph.label2element, s1_vars);

	auto input = make_shared<BindingId>();
	// ObjectId label_type_1 = graph.get_label_id(Label("Muslims"));
	// ObjectId label_type_1 = graph.get_label_id(Label("unico07"));
	// input->add(VarId(1), label_type_1);

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;
	// long limit = 100;//0000000000000;
	while (b != nullptr/* && count <= limit*/) {
		// b->print(var_names);
		// auto n_id = b->search_id(VarId(2));
		// Node node = graph.get_node(n_id->id);

		// auto label_id = b->search_id(VarId(1));
		// Label label = graph.get_label(label_id->id);

		// cout << count << "," << node.get_id() << "," << label.get_label_name() << "\n";
		// cout << b->search_id(VarId(1))->id << "," << b->search_id(VarId(2))->id << "\n";
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_element2label() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Id:"));
	var_names.insert(pair<int, string>(2, "Label:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1));
	s1_vars.push_back(VarId(2));

	GraphScan s1 = GraphScan(graph.graph_id, *graph.element2label, s1_vars);

	auto input = make_shared<BindingId>();

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	while (b != nullptr) {
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_element2prop() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Id:"));
	var_names.insert(pair<int, string>(2, "Key:"));
	var_names.insert(pair<int, string>(3, "Value:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1));
	s1_vars.push_back(VarId(2));
	s1_vars.push_back(VarId(3));

	GraphScan s1 = GraphScan(graph.graph_id, *graph.element2prop, s1_vars);

	auto input = make_shared<BindingId>();

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	while (b != nullptr) {
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_prop2element() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Key"));
	var_names.insert(pair<int, string>(2, "Value"));
	var_names.insert(pair<int, string>(3, "Id"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1));
	s1_vars.push_back(VarId(2));
	s1_vars.push_back(VarId(3));


	GraphScan s1 = GraphScan(graph.graph_id, *graph.prop2element, s1_vars);
	auto input = make_shared<BindingId>();
	// ObjectId key_id = graph.get_key_id(Key("unsung"));
	// input->add(VarId(1), key_id);

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	while (b != nullptr && count < 1000) {
		b->print(var_names);
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_from_to_edge() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "from:"));
	var_names.insert(pair<int, string>(2, "to:"));
	var_names.insert(pair<int, string>(3, "edge:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1));
	s1_vars.push_back(VarId(2));
	s1_vars.push_back(VarId(3));


	GraphScan s1 = GraphScan(graph.graph_id, *graph.from_to_edge, s1_vars);
	auto input = make_shared<BindingId>();

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	while (b != nullptr) {
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_to_from_edge() {
	auto start = std::chrono::system_clock::now();
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "from:"));
	var_names.insert(pair<int, string>(2, "to:"));
	var_names.insert(pair<int, string>(3, "edge:"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(2));
	s1_vars.push_back(VarId(1));
	s1_vars.push_back(VarId(3));


	GraphScan s1 = GraphScan(graph.graph_id, *graph.to_from_edge, s1_vars);
	auto input = make_shared<BindingId>();

	BindingIdIter& root = s1;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;

	while (b != nullptr) {
		b = root.next();
		count++;
	}
	cout << "count: " << count << "\n";
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> duration = end - start;
    std::cout << "time: " << duration.count() << "ms" << std::endl;
}

void TestGraphImport::test_trees() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	cout << "printing label2element:\n";
	test_tree(*graph.label2element);

	cout << "printing element2label:\n";
	test_tree(*graph.element2label);

	cout << "printing prop2element:\n";
	test_tree(*graph.prop2element);

	cout << "printing element2prop:\n";
	test_tree(*graph.element2prop);

	cout << "printing from_to_edge:\n";
	test_tree(*graph.from_to_edge);

	cout << "printing to_from_edge:\n";
	test_tree(*graph.to_from_edge);
}


void TestGraphImport::test_tree(BPlusTree& tree) {
	vector<VarId> vars;
	vars.push_back(VarId(1));
	vars.push_back(VarId(2));
	vars.push_back(VarId(3));

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "first"));
	var_names.insert(pair<int, string>(2, "second"));
	var_names.insert(pair<int, string>(3, "third"));

	GraphScan scan = GraphScan(0, tree, vars);
	auto input = make_shared<BindingId>();

	BindingIdIter& root = scan;
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