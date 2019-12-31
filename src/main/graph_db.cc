#include "tests/tests.h"

#include <climits>
#include <iostream>

int main()
{
	// Next line imports the graph. When running tests comment this line.
	TestGraphImport::bulk_import("test_files/graph_creation_example/nodes.txt", "test_files/graph_creation_example/edges.txt");

	// Uncomment these lines to run tests. Look src/tests/test_query_optimizer.cc to understand what the test does.
	// TestQueryOptimizer::Test1();
	// TestQueryOptimizer::Test2();
	return 0;
}
