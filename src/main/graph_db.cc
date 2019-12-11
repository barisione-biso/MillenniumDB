#include "tests/tests.h"

#include <climits>
// #include <chrono>
// #include <cstdlib>
// #include <climits>
#include <iostream>
// #include <map>
// #include <memory>
// #include <vector>

// void test_nested_loop_join() {
// 	Config config = Config();
// 	RelationalGraph graph = RelationalGraph(0, config);

// 	map<int, string> var_names;
// 	var_names.insert(pair<int, string>(1, "Label:"));
// 	var_names.insert(pair<int, string>(2, "Var:?n"));
// 	var_names.insert(pair<int, string>(3, "Key:"));
// 	var_names.insert(pair<int, string>(4, "Value:?v"));
// 	var_names.insert(pair<int, string>(5, "label:?l"));

// 	vector<VarId> s1_vars;
// 	s1_vars.push_back(VarId(1)); // Label:type1
// 	s1_vars.push_back(VarId(2)); // Node:?n

// 	vector<VarId> s2_vars;
// 	s2_vars.push_back(VarId(2)); // Node:?n
// 	s2_vars.push_back(VarId(3)); // Key:Name
// 	s2_vars.push_back(VarId(4)); // Value:?v

// 	vector<VarId> s3_vars;
// 	s3_vars.push_back(VarId(2)); // Node:?n
// 	s3_vars.push_back(VarId(5)); // Label:?l

// 	GraphScan s1 = GraphScan(graph.graph_id, *graph.label2element, s1_vars);
// 	GraphScan s2 = GraphScan(graph.graph_id, *graph.element2prop, s2_vars);
// 	GraphScan s3 = GraphScan(graph.graph_id, *graph.element2label, s3_vars);

// 	IndexNestedLoopJoin nlj1 = IndexNestedLoopJoin(config, s1, s2);
// 	IndexNestedLoopJoin nlj2 = IndexNestedLoopJoin(config, nlj1, s3);

// 	auto input = make_shared<BindingId>();
// 	ObjectId label_type_1 = graph.get_label_id(Label("baobabs"));
// 	cout << label_type_1.id << "\n";
// 	ObjectId key_name = graph.get_key_id(Key("Omaha"));
// 	input->add(VarId(1), label_type_1);
// 	input->add(VarId(3), key_name);

// 	BindingIdIter& root = nlj2;
// 	root.init(input);
// 	unique_ptr<BindingId const> b = root.next();
// 	int count = 0;
// 	int limit = 100;
// 	while (b != nullptr && ++count <= limit) {
// 		// b->print(var_names);
// 		auto n_id = b->search_id(VarId(2));
// 		Node node = graph.get_node(n_id->id);

// 		auto label_id = b->search_id(VarId(1));
// 		Label label = graph.get_label(label_id->id);

// 		auto key_id = b->search_id(VarId(3));
// 		Key key = graph.get_key(key_id->id);

// 		auto value_id = b->search_id(VarId(4));
// 		auto value = graph.get_value(value_id->id);

// 		auto label2_id = b->search_id(VarId(5));
// 		Label label2 = graph.get_label(label2_id->id);

// 		cout << count << ") NodeId: " << node.get_id();
// 		cout << ":" << label.get_label_name() << "\t";
// 		cout << ":" << label2.get_label_name() << "\t";
// 		cout << key.get_key_name() << ": " << value->to_string() << "\t";
// 		cout << "\n";
// 		b = root.next();
// 	}
// }

// void test_ordered_file() {
// 	OrderedFile ordered_file = OrderedFile("ordered_file.bin", 3);
// 	uint64_t* c = new uint64_t[3];
// 	std::vector<uint_fast8_t> column_order;
// 	column_order.push_back(1);
// 	column_order.push_back(0);
// 	column_order.push_back(2);

// 	cout << "> Insertando records\n";
// 	auto start = std::chrono::system_clock::now();
// 	for (uint64_t i = 0; i < 10'000; i++) {
// 		c[0] = (uint64_t) rand();
// 		for (uint64_t j = 0; j < 1'000; j++) {
// 			c[1] = (uint64_t) rand();
// 			ordered_file.append_record(Record(c[1], c[0], i*1'000 + j));
// 		}
// 		// c[2] = (uint64_t) rand();
// 		// cout << "> Insertando record " << i << ": (" << (uint64_t)c[0] << ", " << (uint64_t)c[1] << ")\n";
// 	}
// 	delete[] c;
// 	auto end1 = std::chrono::system_clock::now();
// 	std::chrono::duration<float,std::milli> duration = end1 - start;
//     std::cout << duration.count() << "ms " << std::endl;

// 	std::cout << "Ordering...\n";
// 	ordered_file.order(column_order);
// 	// ordered_file.print();
// 	ordered_file.check_order(column_order);

// 	/**********/
// 	BPlusTreeParams bpt_params = BPlusTreeParams("example_bpt", 3);
//     BPlusTree bpt = BPlusTree(bpt_params);
// 	bpt.bulk_import(ordered_file);
// }

// void test_bpts_from_import() {
// 	BPlusTreeParams label2element_params = BPlusTreeParams("label2element", 2);
//     BPlusTree label2element = BPlusTree(label2element_params);
// 	search_records(label2element);

// 	BPlusTreeParams element2label_params = BPlusTreeParams("element2label", 2);
//     BPlusTree element2label = BPlusTree(element2label_params);
// 	search_records(element2label);
// }

int main()
{
	// TestGraphImport::bulk_import("test_files/graph_creation_big/nodes.txt", "test_files/graph_creation_big/edges.txt");
	TestGraphImport::bulk_import("test_files/graph_creation_small/nodes.txt", "test_files/graph_creation_small/edges.txt");
	// TestGraphImport::test_label2element();
	// TestGraphImport::test_element2label();
	// TestGraphImport::test_prop2element();
	// TestGraphImport::test_element2prop();
	// TestGraphImport::test_to_from_edge();
	// TestGraphImport::test_from_to_edge();

	TestGraphImport::test_trees();

	// TestBPlusTree::create();
	// TestBPlusTree::test_order();
	return 0;
}
