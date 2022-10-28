#pragma once

#include "execution/binding_id_iter/paths/path_index.h"

template <std::size_t N> class BptIter;

namespace Paths {
/*
B+Tree index iterator.
Assumes B+Trees with ids[2] as the result of interest:
    QuadModel: (type, from/to, to/from, edge) -> ids[2] = to/from
    RDF: (S,P,O) -> ids[2] = O
    RDF: (P,O,S) -> ids[2] = S
*/
class BTreePathIndexIter : public PathIndexIter {
private:
    // B+Tree internal iterator
    std::unique_ptr<BptIter<3>> iter;

    // Current result
    uint64_t current;

    // Whether the iterator is finished or not
    bool finished = false;

public:
    BTreePathIndexIter(std::unique_ptr<BptIter<3>> iter);

    // Interface
    uint64_t get() override;
    bool next() override;
    bool at_end() override;
};
}