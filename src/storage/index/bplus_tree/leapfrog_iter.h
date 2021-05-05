#ifndef STORAGE__LEAPFROG_ITER_H_
#define STORAGE__LEAPFROG_ITER_H_

#include <stack>
#include <vector>

#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_dir.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/tuple_buffer/tuple_buffer.h"

class TupleBuffer;

class LeapfrogIter {
public:
    virtual ~LeapfrogIter() = default;

    virtual void up() = 0; // TODO: podría no ser virtual?
    virtual void down() = 0;
    virtual bool open_terms() = 0;
    virtual bool next() = 0;
    virtual bool seek(uint64_t key) = 0;
    virtual uint64_t get_key() const = 0;

    virtual const std::vector<VarId>& get_intersection_vars() = 0; // TODO: podría no ser virtual?
    virtual const std::vector<VarId>& get_enumeration_vars() = 0;  // TODO: podría no ser virtual?

    // will consume all tuples and write them into the buffer+
    virtual void enum_no_intersection(TupleBuffer& buffer) = 0;
};


template <std::size_t N>
class LeapfrogIterImpl : public LeapfrogIter {
public:
    LeapfrogIterImpl(const BPlusTree<N>& btree,
                     const std::vector<ObjectId> terms,
                     const std::vector<VarId> intersection_vars,
                     const std::vector<VarId> enumeration_vars);

    ~LeapfrogIterImpl() = default;

    inline void up() override { level--; }
    inline uint64_t get_key() const override { return (*current_tuple)[level]; }
    inline const std::vector<VarId>& get_intersection_vars() override { return intersection_vars; }
    inline const std::vector<VarId>& get_enumeration_vars() override { return enumeration_vars; }

    // Increases the level and sets the current_tuple
    void down() override;

    // Sets the current tuple with the next record at the current level
    // not the same concept of a the next in the B+Tree, because may skip many records
    // returns false if there is no next record
    bool next() override;

    bool next_enumeration();

    // Sets the current tuple with the a record that has a greater or equal key at the current level
    // returns false if there is no such record
    bool seek(uint64_t key) override;

    void enum_no_intersection(TupleBuffer& buffer) override;

    bool open_terms() override;

private:
    // const BPlusTree<N>& btree;

    const std::vector<ObjectId> terms;

    const std::vector<VarId> intersection_vars;

    const std::vector<VarId> enumeration_vars;

    std::unique_ptr<Record<N>> current_tuple;

    int_fast32_t level; // can go from -1 to N-1

    std::unique_ptr<BPlusTreeLeaf<N>> current_leaf;
    uint32_t current_pos_in_leaf;

    std::stack<std::unique_ptr<BPlusTreeDir<N>>> directory_stack;

    // search a record in the interval [min, max]
    bool internal_search(const Record<N>& min, const Record<N>& max);
};

#endif // STORAGE__LEAPFROG_ITER_H_
