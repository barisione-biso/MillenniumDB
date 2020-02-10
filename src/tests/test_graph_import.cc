#include "tests/test_graph_import.h"

#include "base/var/var_id.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/import/bulk_import.h"
#include "relational_model/binding/binding_id.h"
#include "relational_model/physical_plan/binding_id_iter/graph_scan.h"
#include "relational_model/physical_plan/binding_id_iter/index_nested_loop_join.h"

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
}

void TestGraphImport::test_element2label() {
}

void TestGraphImport::test_element2prop() {
}

void TestGraphImport::test_from_to_edge() {
}

void TestGraphImport::test_to_from_edge() {
}

void TestGraphImport::test_trees() {
	Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
}


void TestGraphImport::test_tree(BPlusTree&) {
}