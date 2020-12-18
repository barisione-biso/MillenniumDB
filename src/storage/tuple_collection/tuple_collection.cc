#include "tuple_collection.h"

#include <cassert>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "storage/file_id.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"

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
    const int bytes_used = (*tuple_count) * tuple_size * GRAPH_OBJECT_SIZE + sizeof(tuple_count);
    const int size_new_tuple = tuple_size * GRAPH_OBJECT_SIZE;
    if (bytes_used + size_new_tuple < Page::PAGE_SIZE) {
      return false;
    }
    return true;
}

uint64_t TupleCollection::get_n_tuples() const {
  return *tuple_count;
}


void TupleCollection::add(GraphObject* new_tuple) {
    const size_t bytes_used = (*tuple_count) * tuple_size * GRAPH_OBJECT_SIZE;
    for (size_t i = 0; i < tuple_size * GRAPH_OBJECT_SIZE; i++) {
        tuples[bytes_used + i] = new_tuple[i];
    }
  	(*tuple_count)++;
}


uint8_t* TupleCollection::get(uint_fast64_t id) const {
    uint8_t n_tuple[GRAPH_OBJECT_SIZE * tuple_size];
    size_t tuple_position = id * GRAPH_OBJECT_SIZE * tuple_size;
    for (size_t i = 0; i < tuple_size * GRAPH_OBJECT_SIZE; i++) {
        n_tuple[i] = tuples[tuple_position + i];
    }
    return n_tuple;
}


void TupleCollection::override_tuple(uint8_t* tuple_to_override, int position) {
    size_t position_to_override = position * GRAPH_OBJECT_SIZE * tuple_size;
    for (size_t i = 0; i < tuple_size * GRAPH_OBJECT_SIZE; i++) {
        tuples[position_to_override + i] = tuple_to_override[i];
    }
}


void TupleCollection::reset() {
    (*tuple_count) = 0;
}


void TupleCollection::swap(int x, int y) {
    uint8_t* x_tuple = get(x);
    uint8_t* y_tuple = get(y);
    override_tuple(x_tuple, y);
    override_tuple(y_tuple, x);
}


void TupleCollection::sort( bool (*has_priority)(uint8_t* lhs, uint8_t* rhs, std::vector<uint64_t> order_vars),std::vector<uint64_t> order_vars) {
    quicksort(0, (*tuple_count) - 1, has_priority, order_vars);
    /*
    for (size_t i = 0; i < *tuple_count - 1; i++) {
      for (size_t j = i + 1; j < *tuple_count; j++) {
        if (has_priority(get(j), get(i), order_vars)) {
          swap(i,j);
        }
      }
    }
    */
}


void TupleCollection::quicksort(int i, int f, bool (*has_priority)(uint8_t* x, uint8_t* y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars) {
    if (i < f) {
        int p = partition(i, f, has_priority, order_vars);
        quicksort(i, p - 1, has_priority, order_vars);
        quicksort(p + 1, f, has_priority, order_vars);
    }
}


int TupleCollection::partition(int i, int f, bool (*has_priority)(uint8_t* x, uint8_t* y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars) {
    int x = i + (rand() % (f - i + 1));
    uint8_t* p = get(x);
    TupleCollection::swap(x,f);
    int low_el = i - 1;
    for (int j = i; j <= f - 1; j++) {
      if (has_priority(get(j), p, order_vars)) {
        low_el++;
        TupleCollection::swap(low_el, j);
      }
    }
    TupleCollection::swap(low_el + 1, f);
    return low_el + 1;
}

MergeOrderedTupleCollection::MergeOrderedTupleCollection(
    size_t tuple_size,
    std::vector<uint_fast64_t> order_vars,
    bool (*has_priority)(uint8_t* lhs, uint8_t* rhs, std::vector<uint64_t> order_vars)) :
        tuple_size(tuple_size),
        order_vars(order_vars),
        has_priority(has_priority) { }


void MergeOrderedTupleCollection::merge(
    uint_fast64_t left_start,
    uint_fast64_t left_end,
    uint_fast64_t right_start,
    uint_fast64_t right_end,
    FileId source_file_id,
    FileId output_file_id
    ) {
    auto left_run = make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, left_start), tuple_size);
    auto right_run = make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, right_start), tuple_size);
    auto out_run = make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, left_start), tuple_size);
    out_run->reset();
    left_tuple = left_run->get(0);
    right_tuple = right_run->get(0);
    uint_fast64_t left_counter = 0;
    uint_fast64_t right_counter = 0;
    uint_fast64_t out_page_counter = left_start;
    bool left_first;
    bool open_left = true;
    bool open_right = true;
    while (open_left || open_right) {
        if (out_run->is_full()) {
            out_page_counter++;
            out_run = make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, out_page_counter), tuple_size);
            out_run->reset();
        }
        left_first = has_priority(left_tuple, right_tuple, order_vars);
        if (open_left && (left_first || !open_right)) {
            out_run->add(left_tuple);
            left_counter++;
            if (left_counter == left_run->get_n_tuples()) {
                left_start++;
                if (left_start <= left_end) {
                    left_run = make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, left_start), tuple_size);
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
            if (right_counter == right_run->get_n_tuples()) {
                right_start++;
                if (right_start <= right_end) {
                    right_run = make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, right_start), tuple_size);
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


void MergeOrderedTupleCollection::copy_page(
      uint_fast64_t source_page,
      FileId source_file_id,
      FileId output_file_id
    ) {
      auto source_tuples = make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, source_page), tuple_size);
      auto output_tuples = make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, source_page), tuple_size);
      output_tuples->reset();
      for (size_t i = 0; i < source_tuples->get_n_tuples(); i++) {
          uint8_t* t = source_tuples->get(i);
          output_tuples->add(t);
      }
      source_tuples->reset();
}
