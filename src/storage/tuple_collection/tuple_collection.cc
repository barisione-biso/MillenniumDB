#include "tuple_collection.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "storage/buffer_manager.h"
#include "storage/file_id.h"
#include "storage/file_manager.h"

using namespace std;

TupleCollection::TupleCollection(Page& page, const size_t tuple_size) :
    page(page),
    tuple_size(tuple_size)
{
    tuple_count = reinterpret_cast<uint64_t*>(page.get_bytes());
    tuples = reinterpret_cast<GraphObject*>(page.get_bytes() + sizeof(tuple_count));
}


TupleCollection::~TupleCollection() {
    page.make_dirty();
    buffer_manager.unpin(page);
}


bool TupleCollection::is_full() const {
    const auto bytes_used = (*tuple_count) * tuple_size * GRAPH_OBJECT_SIZE + sizeof(tuple_count);
    const auto size_new_tuple = tuple_size * GRAPH_OBJECT_SIZE;
    // if (bytes_used + size_new_tuple < Page::PAGE_SIZE) {
    //     return false;
    // }
    // return true;
    return bytes_used + size_new_tuple >= Page::PAGE_SIZE;
}


void TupleCollection::add(std::vector<GraphObject> new_tuple) {
    // Add a new tuple in the last position of the page
    const size_t bytes_used = (*tuple_count) * tuple_size;
    for (size_t i = 0; i < tuple_size; i++) {
        tuples[bytes_used + i] = new_tuple[i];
    }
  	(*tuple_count)++;
}


std::vector<GraphObject> TupleCollection::get(uint_fast64_t id) const {
    // Return the n-th tuple of the page
    std::vector<GraphObject> res(tuple_size);
    size_t tuple_position = id * tuple_size;
    for (size_t i = 0; i < tuple_size; i++) {
        res[i] = tuples[tuple_position + i];
    }
    return res;
}


void TupleCollection::override_tuple(std::vector<GraphObject> tuple_to_override, int position) {
    // Write a tupĺe in the specific position. tuple_count don't increase because
    // this method assume that is overriding a tuple
    size_t position_to_override = position * tuple_size;
    for (size_t i = 0; i < tuple_size; i++) {
        tuples[position_to_override + i] = tuple_to_override[i];
    }
}


void TupleCollection::reset() {
    // Causes all tuples on the page will be ignored
    (*tuple_count) = 0;
}


void TupleCollection::swap(int x, int y) {
    // Override tuple in position x in position y
    // and tuple in position y in position x
    auto x_tuple = get(x);
    auto y_tuple = get(y);
    override_tuple(x_tuple, y);
    override_tuple(y_tuple, x);
}


void TupleCollection::sort(std::vector<uint_fast64_t>& order_vars, std::vector<bool>& ascending) {
    // Sort all the tuples. order_vars the position of the vars in each array that
    // define the order.
    quicksort(0, (*tuple_count) - 1, order_vars, ascending);

    // Insertion sort:
    // for (size_t i = 0; i < *tuple_count - 1; i++) {
    //     for (size_t j = i + 1; j < *tuple_count; j++) {
    //         if (TupleCollection::has_priority(get(j), get(i), order_vars, ascending)) {
    //             swap(i,j);
    //         }
    //     }
    // }
}


bool TupleCollection::has_priority(std::vector<GraphObject> lhs, std::vector<GraphObject> rhs,
                                   std::vector<uint_fast64_t>& order_vars, std::vector<bool>& ascending)
{
    assert(order_vars.size() == ascending.size());
    for (size_t i = 0; i < order_vars.size(); i++) {
        if (ascending[i]) {
            if (lhs[order_vars[i]] < rhs[order_vars[i]]) {
                return true;
            } else if (rhs[order_vars[i]] < lhs[order_vars[i]]) {
                return false;
            }
        } else {
            if (lhs[order_vars[i]] > rhs[order_vars[i]]) {
                return true;
            } else if (rhs[order_vars[i]] > lhs[order_vars[i]]) {
                return false;
            }
        }
    }
    return true;
}


void TupleCollection::quicksort(int i, int f, std::vector<uint_fast64_t>& order_vars, std::vector<bool>& ascending) {
    if (i < f) {
        int p = partition(i, f, order_vars, ascending);
        quicksort(i, p - 1, order_vars, ascending);
        quicksort(p + 1, f, order_vars, ascending);
    }
}


int TupleCollection::partition(int i, int f, std::vector<uint_fast64_t>& order_vars, std::vector<bool>& ascending) {
    int x = i + (rand() % (f - i + 1));
    auto p = get(x);
    swap(x, f);
    int low_el = i - 1;
    for (int j = i; j <= f - 1; j++) {
        if (has_priority(get(j), p, order_vars, ascending)) {
            low_el++;
            swap(low_el, j);
        }
    }
    swap(low_el + 1, f);
    return low_el + 1;
}


MergeOrderedTupleCollection::MergeOrderedTupleCollection(size_t tuple_size,
                                                         std::vector<uint_fast64_t> order_vars,
                                                         std::vector<bool> ascending) :
    tuple_size (tuple_size),
    order_vars (order_vars),
    ascending  (ascending) { }


void MergeOrderedTupleCollection::merge(uint_fast64_t left_start,
                                        uint_fast64_t left_end,
                                        uint_fast64_t right_start,
                                        uint_fast64_t right_end,
                                        TmpFileId source_file_id,
                                        TmpFileId output_file_id)
{
    // Merge [left_start-left_end] with [right_start-right_end] from source_file_id in output_file_id,
    // in output_file_id is sorted
    auto left_run  = make_unique<TupleCollection>(buffer_manager.get_tmp_page(source_file_id, left_start),  tuple_size);
    auto right_run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(source_file_id, right_start), tuple_size);
    auto out_run   = make_unique<TupleCollection>(buffer_manager.get_tmp_page(output_file_id, left_start),  tuple_size);
    out_run->reset();

    auto left_tuple  = left_run->get(0);
    auto right_tuple = right_run->get(0);
    uint_fast64_t left_counter = 0;
    uint_fast64_t right_counter = 0;
    uint_fast64_t out_page_counter = left_start;
    bool left_first;
    bool open_left = true;
    bool open_right = true;

    while (open_left || open_right) {
        if (out_run->is_full()) {
            out_page_counter++;
            out_run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(output_file_id, out_page_counter), tuple_size);
            out_run->reset();
        }
        left_first = TupleCollection::has_priority(left_tuple, right_tuple, order_vars, ascending);
        if (open_left && (left_first || !open_right)) {
            out_run->add(left_tuple);
            left_counter++;
            if (left_counter == left_run->get_tuple_count()) {
                left_start++;
                if (left_start <= left_end) {
                    left_run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(source_file_id, left_start), tuple_size);
                    left_counter = 0;
                } else {
                    open_left = false;
                    continue;
                }
            }
            left_tuple = left_run->get(left_counter);
        }
        else if (open_right && (!left_first || !open_left)) {
            out_run->add(right_tuple);
            right_counter++;
            if (right_counter == right_run->get_tuple_count()) {
                right_start++;
                if (right_start <= right_end) {
                    right_run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(source_file_id, right_start), tuple_size);
                    right_counter = 0;
                } else {
                    open_right = false;
                    continue;
                }
            }
            right_tuple = right_run->get(right_counter);
        }
    }
}


void MergeOrderedTupleCollection::copy_page(uint_fast64_t source_page,
                                            TmpFileId source_file_id,
                                            TmpFileId output_file_id)
{
    auto source_tuples = make_unique<TupleCollection>(buffer_manager.get_tmp_page(source_file_id, source_page), tuple_size);
    auto output_tuples = make_unique<TupleCollection>(buffer_manager.get_tmp_page(output_file_id, source_page), tuple_size);
    output_tuples->reset();
    for (size_t i = 0; i < source_tuples->get_tuple_count(); i++) {
        auto t = source_tuples->get(i);
        output_tuples->add(t);
    }
    source_tuples->reset();
}
