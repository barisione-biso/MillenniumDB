#include "btree_path_index_iter.h"

#include "storage/index/bplus_tree/bplus_tree.h"

using namespace Paths;
using namespace std;

// B+Tree
BTreePathIndexIter::BTreePathIndexIter(unique_ptr<BptIter<3>> iter) :
    iter (move(iter)) {}


uint64_t BTreePathIndexIter::get() {
    return current;
}


bool BTreePathIndexIter::next() {
    // Don't do anything if already finished
    if (finished) {
        return false;
    }

    // Advance iterator
    auto next_result = iter->next();
    if (next_result != nullptr) {
        current = next_result->ids[2];
        return true;
    }

    // Mark as finished
    finished = true;
    return false;
}


bool BTreePathIndexIter::at_end() {
    return finished;
}