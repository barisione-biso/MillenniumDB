#include "base/var/var_id.h"
#include "base/graph/value/value_string.h"
#include "file/index/object_file/object_file.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/operators/index_nested_loop_join.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <vector>


using namespace std;

void insert_records(BPlusTree& bpt) {
	// for (int i = 0; i < 50; i++) {
	// 	uint64_t* c = new uint64_t[2];
	// 	c[0] = (uint64_t) rand()/1000;
	// 	c[1] = (uint64_t) rand()/1000;
	// 	cout << "> Insertando record (" << (int)c[0] << ", " << (int)c[1] << ")\n";
	// 	Record record = Record(c, 2);
	// 	bpt.insert(record);
	// 	delete[] c;
	// }
	for (int i = 0; i < 150; i++) {
		uint64_t* c = new uint64_t[2];
		c[0] = (uint64_t) rand()/1000;
		c[1] = (uint64_t) rand()/1000;
		cout << "> Insertando record (" << (int)c[0] << ", " << (int)c[1] << ")\n";

		bpt.insert(Record(c[0]), Record(c[1]));
		delete[] c;
	}
}

void search_records(BPlusTree& bpt) {
	 uint64_t min[] = {0, 0};
	//uint64_t min[] = {628175,  1656478};
	//uint64_t max[] = {2044897, 1967514};
	uint64_t max[] = {9992044897, 9991967514};
	auto it = bpt.get_range(Record(min, 2), Record(max, 2));
	auto record = it->next();
	while (record != nullptr) {
		cout << "(" << (int)record->ids[0] << ", " << (int)record->ids[1] << ")\n";
		// bpt.edit(Record(record->ids[0]), Record(record->ids[0]+1));
		// cout << "(" << bpt.get(Record(record->ids[0]))->ids[0] << ")\n";
		record = it->next();
	}
}

void test_bpt() {
	BufferManager buffer_manager = BufferManager();
	BPlusTreeParams bpt_params = BPlusTreeParams(buffer_manager, "test_files/example_bpt", 1, 1);
    BPlusTree bpt = BPlusTree(bpt_params);
	insert_records(bpt);
	cout << "Records insertados!\n";
	search_records(bpt);
}

void test_nested_loop_join() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Label:type1"));
	var_names.insert(pair<int, string>(2, "Var:?n"));
	var_names.insert(pair<int, string>(3, "Key:Name"));
	var_names.insert(pair<int, string>(4, "Value:?v"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1)); // Label:type1
	s1_vars.push_back(VarId(2)); // Node:?n

	vector<VarId> s2_vars;
	s2_vars.push_back(VarId(2)); // Node:?n
	s2_vars.push_back(VarId(3)); // Key:Name
	s2_vars.push_back(VarId(4)); // Value:?v

	GraphScan s1 = GraphScan(graph.graph_id, *graph.label2element, s1_vars);
	GraphScan s2 = GraphScan(graph.graph_id, *graph.prop2element, s2_vars);

	IndexNestedLoopJoin nlj = IndexNestedLoopJoin(config, s1, s2);

	auto input = make_shared<BindingId>();
	ObjectId label_type_1 = graph.get_label_id(Label("type1"));
	ObjectId key_name = graph.get_key_id(Key("Name"));
	input->add(VarId(1), label_type_1);
	input->add(VarId(3), key_name);

	BindingIdIter& root = nlj;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	while (b != nullptr) {
		b->print(var_names);
		// auto n_id = b->search_id(VarId(2));
		// Node node = graph.get_node(n_id->id);

		// auto label_id = b->search_id(VarId(1));
		// Label label = graph.get_label(label_id->id);

		// auto key_id = b->search_id(VarId(3));
		// Key key = graph.get_key(key_id->id);

		// auto value_id = b->search_id(VarId(4));
		// auto value = graph.get_value(value_id->id);

		// cout << "Node:  " << node.get_id();
		// cout << "[" << label.get_label_name() << "]\t";
		// cout << key.get_key_name() << ": " << value->to_string() << "\t";
		// cout << "\n";
		b = root.next();
	}
}


void test_graph_creation() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);

	for (u_int64_t i = 0; i < 10; i++) {
		Node n = graph.create_node("N" + to_string(i));
		graph.add_label(n, Label("type1"));
		Key key = Key("Name");
		ValueString value = ValueString("name" + to_string((int)i));
		graph.add_property(n, Property(key, value));
	}

	for (u_int64_t i = 0; i < 10; i++) {
		Node n = graph.create_node("N" + to_string(10+i));
		graph.add_label(n, Label("type2"));
		Key key = Key("Name");
		ValueString value = ValueString("name" + to_string((int)i));
		graph.add_property(n, Property(key, value));
	}
}

int main()
{
	// test_graph_creation();
	// cout << "Graph created\n";
	// test_nested_loop_join();
	test_bpt();
	return 0;
}
