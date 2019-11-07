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
#include "relational_model/import/bulk_import.h"

#include <cstdlib>
#include <climits>
#include <iostream>
#include <map>
#include <memory>
#include <vector>


using namespace std;

void insert_records(BPlusTree& bpt) {
	for (int i = 0; i < 100000; i++) {
		uint64_t* c = new uint64_t[2];
		c[0] = (uint64_t) rand();
		c[1] = (uint64_t) rand();
		// cout << "> Insertando record " << i << ": (" << (int)c[0] << ", " << (int)c[1] << ")\n";

		bpt.insert(Record(c[0], c[1]));
		delete[] c;
	}
}

void search_records(BPlusTree& bpt) {
	uint64_t min[] = {0, 0};
	uint64_t max[] = {ULONG_MAX, ULONG_MAX};
	//uint64_t min[] = {628175,  1656478};
	//uint64_t max[] = {2044897, 1967514};
	auto it = bpt.get_range(Record(min[0], min[1]), Record(max[0], max[1]));
	auto record = it->next();
	int i = 1;
	while (record != nullptr) {
		cout << i++ << ": (" << (int)record->ids[0] << ", " << (int)record->ids[1] << ")\n";
		// bpt.edit(Record(record->ids[0]), Record(record->ids[0]+1));
		// cout << "(" << bpt.get(Record(record->ids[0]))->ids[0] << ")\n";
		record = it->next();
	}
}

void test_bpt() {
	BufferManager buffer_manager = BufferManager();
	BPlusTreeParams bpt_params = BPlusTreeParams(buffer_manager, "test_files/example_bpt", 2);
    BPlusTree bpt = BPlusTree(bpt_params);
	insert_records(bpt);
	search_records(bpt);
}

void test_nested_loop_join() {
	cout << "Testing nested loop join\n";
	Config config = Config();
	cout << "Config loaded\n";
	RelationalGraph graph = RelationalGraph(0, config);
	cout << "Graph initialized\n";

	map<int, string> var_names;
	var_names.insert(pair<int, string>(1, "Label:Person"));
	var_names.insert(pair<int, string>(2, "Var:?n"));
	var_names.insert(pair<int, string>(3, "Key:name"));
	var_names.insert(pair<int, string>(4, "Value:?v"));
	var_names.insert(pair<int, string>(5, "label:?l"));

	vector<VarId> s1_vars;
	s1_vars.push_back(VarId(1)); // Label:type1
	s1_vars.push_back(VarId(2)); // Node:?n

	vector<VarId> s2_vars;
	s2_vars.push_back(VarId(2)); // Node:?n
	s2_vars.push_back(VarId(3)); // Key:Name
	s2_vars.push_back(VarId(4)); // Value:?v

	vector<VarId> s3_vars;
	s3_vars.push_back(VarId(2)); // Node:?n
	s3_vars.push_back(VarId(5)); // Label:?l

	GraphScan s1 = GraphScan(graph.graph_id, *graph.label2element, s1_vars);
	GraphScan s2 = GraphScan(graph.graph_id, *graph.element2prop, s2_vars);
	GraphScan s3 = GraphScan(graph.graph_id, *graph.element2label, s3_vars);

	IndexNestedLoopJoin nlj1 = IndexNestedLoopJoin(config, s1, s2);
	IndexNestedLoopJoin nlj2 = IndexNestedLoopJoin(config, nlj1, s3);

	auto input = make_shared<BindingId>();
	ObjectId label_type_1 = graph.get_label_id(Label("moribund"));
	ObjectId key_name = graph.get_key_id(Key("buddy"));
	input->add(VarId(1), label_type_1);
	input->add(VarId(3), key_name);

	BindingIdIter& root = nlj2;
	root.init(input);
	unique_ptr<BindingId const> b = root.next();
	int count = 0;
	int limit = 100;
	while (b != nullptr && ++count <= limit) {
		// b->print(var_names);
		auto n_id = b->search_id(VarId(2));
		Node node = graph.get_node(n_id->id);

		auto label_id = b->search_id(VarId(1));
		Label label = graph.get_label(label_id->id);

		auto key_id = b->search_id(VarId(3));
		Key key = graph.get_key(key_id->id);

		auto value_id = b->search_id(VarId(4));
		auto value = graph.get_value(value_id->id);

		auto label2_id = b->search_id(VarId(5));
		Label label2 = graph.get_label(label2_id->id);

		cout << count << ") NodeId: " << node.get_id();
		cout << ":" << label.get_label_name() << "\t";
		cout << ":" << label2.get_label_name() << "\t";
		cout << key.get_key_name() << ": " << value->to_string() << "\t";
		cout << "\n";
		b = root.next();
	}
}

void test_bulk_import() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
	cout << "Graph initialized\n";
	BulkImport import = BulkImport("test_files/graph_creation_1/nodes.txt", "test_files/graph_creation_1/edges.txt", graph);
	cout << "Starting import\n";
	import.start_import();
}

int main()
{
	// test_bulk_import();
	test_nested_loop_join();
	// test_bpt();
	return 0;
}
