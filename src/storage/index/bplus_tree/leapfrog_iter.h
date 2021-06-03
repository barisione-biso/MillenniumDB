#ifndef STORAGE__LEAPFROG_ITER_H_
#define STORAGE__LEAPFROG_ITER_H_

#include <stack>
#include <vector>

#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_dir.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/tuple_buffer/tuple_buffer.h"

class TupleBuffer;

class LeapfrogIter {
public:
    virtual ~LeapfrogIter() = default;

    virtual void up() { level--; }
    virtual void down() = 0;
    virtual bool open_terms(BindingId& input_binding) = 0;
    virtual bool next() = 0;
    virtual bool seek(uint64_t key) = 0;
    virtual uint64_t get_key() const = 0;

    inline const std::vector<VarId>& get_intersection_vars() { return intersection_vars; }
    inline const std::vector<VarId>& get_enumeration_vars()  { return enumeration_vars; }

    // will consume all tuples and write them into the buffer. Invalidates the current_leaf
    virtual void enum_no_intersection(TupleBuffer& buffer) = 0;

protected:
    const std::vector<std::unique_ptr<ScanRange>> initial_ranges;
    const std::vector<VarId> intersection_vars;
    const std::vector<VarId> enumeration_vars;

    int_fast32_t level = -1; // can go from -1 to N-1

    LeapfrogIter(std::vector<std::unique_ptr<ScanRange>> _initial_ranges,
                 std::vector<VarId>                      _intersection_vars,
                 std::vector<VarId>                      _enumeration_vars) :
        initial_ranges    (std::move(_initial_ranges)),
        intersection_vars (std::move(_intersection_vars)),
        enumeration_vars  (std::move(_enumeration_vars)) { }
};


template <std::size_t N>
class LeapfrogIterImpl : public LeapfrogIter {
public:
    LeapfrogIterImpl(const BPlusTree<N>&                           btree,
                     const std::vector<std::unique_ptr<ScanRange>> initial_ranges,
                     const std::vector<VarId>                      intersection_vars,
                     const std::vector<VarId>                      enumeration_vars);

    ~LeapfrogIterImpl() = default;

    inline uint64_t get_key() const override { return (*current_tuple)[level]; }

    // Increases the level and sets the current_tuple
    void down() override;

    // Sets the current tuple with the next record at the current level
    // not the same concept of a the next in the B+Tree, because may skip many records
    // returns false if there is no next record
    bool next() override;

    bool next_enumeration();

    // Sets the current tuple with a record that has a greater or equal key at the current level
    // returns false if there is no such record
    bool seek(uint64_t key) override;

    void enum_no_intersection(TupleBuffer& buffer) override;


    // returns true if the terms and parent_binding were found
    bool open_terms(BindingId& input_binding) override;

private:
    std::unique_ptr<Record<N>> current_tuple;

    std::unique_ptr<BPlusTreeLeaf<N>> current_leaf;

    uint32_t current_pos_in_leaf;

    std::stack<std::unique_ptr<BPlusTreeDir<N>>> directory_stack;

    // search a record in the interval [min, max]
    bool internal_search(const Record<N>& min, const Record<N>& max);
};

#endif // STORAGE__LEAPFROG_ITER_H_
