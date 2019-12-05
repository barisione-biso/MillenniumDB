#include "tests/test_bplus_tree.h"

#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include <climits>

void TestBPlusTree::create() {
    BPlusTreeParams bpt_params = BPlusTreeParams("test_bpt", 2);
    BPlusTree bpt = BPlusTree(bpt_params);

    for (int i = 0; i < 100'000; i++) {
        uint64_t* c = new uint64_t[2];
        c[0] = (uint64_t) rand();
        c[1] = (uint64_t) rand();

        bpt.insert(Record(c[0], c[1]));
        delete[] c;
    }
    std::cout << "bpt created\n";
}

void TestBPlusTree::test_order() {
    BPlusTreeParams bpt_params = BPlusTreeParams("test_bpt", 2);
    BPlusTree bpt = BPlusTree(bpt_params);

    uint64_t min[] = {0, 0};
	uint64_t max[] = {ULONG_MAX, ULONG_MAX};
	auto it = bpt.get_range(Record(min[0], min[1]), Record(max[0], max[1]));
	auto x = it->next();
	auto y = it->next();
    int i = 1;
	while (y != nullptr) {
        i++;
        if (*y <= *x) {
            std::cout << "error en el orden de la tuplas " << (i-1) << " y " << (i) << "\n";
        }
		x = std::move(y);
        y = it->next();
	}
    std::cout << "Orden de las " << i << " tuplas correcto.\n";
}