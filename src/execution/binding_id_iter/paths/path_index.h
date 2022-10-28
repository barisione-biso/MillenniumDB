#pragma once

#include <memory>

namespace Paths {

// Index types
enum class IndexType {
    BTREE,     // B+Tree
    // TRIE,      // Trie
    // HASH_TRIE  // Hash Trie
};

/*
Index iterator to explore results.
*/
class PathIndexIter {
public:
    virtual ~PathIndexIter() = default;

    // Iterator interface

    // Obtain current result
    // Calling get() before the first next() or after the iterator is at_end() gives undefined behaviour
    virtual uint64_t get() = 0;

    // Advance iterator and return true if there is a next result, else do nothing and return false
    virtual bool next() = 0;

    // Check if the iterator has finished (no more results)
    virtual bool at_end() = 0;
};

/*
Index for a specific transition.
Given a current node_id, it returns an iterator for all results that satisfy the transition.
*/
class PathIndex {
public:
    virtual ~PathIndex() = default;

    // Get iterator for current results
    virtual std::unique_ptr<PathIndexIter> get_iterator(uint64_t node_id) = 0;
};

/*
Provides an index for each transition (considering the edge type and direction).
In-memory indexes are materialized after calling the materialize() method.
*/
class PathIndexProvider {
public:
    virtual ~PathIndexProvider() = default;

    // Get iterator for a transition defined by an edge type and direction, given the current node
    virtual std::unique_ptr<PathIndexIter> get_iterator(uint64_t type_id, bool inverse, uint64_t node_id) = 0;

    // Check if a node exists in the database (using B+Tree)
    virtual bool node_exists(uint64_t node_id) = 0;
};

/*
Default index iterator.
*/
class NullIndexIter : public PathIndexIter {
public:
    // Interface
    uint64_t get() override {return 0;}
    bool next() override {return false;}
    bool at_end() override {return true;}
};

} // namespace Paths
