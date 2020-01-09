#include "tests/tests.h"

#include <climits>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 3) {
        std::cerr << "Error: expected node and edge files.\n";
        return 1;
    }
	// TODO: move import away from tests to its own module
	TestGraphImport::bulk_import(argv[1], argv[2]);
	return 0;
}
