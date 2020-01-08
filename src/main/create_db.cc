#include "tests/tests.h"

#include <climits>
#include <iostream>

int main(int argc, char *argv[])
{
	// TODO: get names from argv
	// TODO: move import away from tests to its own module
	// Next line imports the graph. When running tests comment this line.
	TestGraphImport::bulk_import("test_files/graph_creation_example/nodes.txt", "test_files/graph_creation_example/edges.txt");
	return 0;
}
