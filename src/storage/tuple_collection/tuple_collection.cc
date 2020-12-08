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
    tuples = reinterpret_cast<uint64_t*>(page.get_bytes() + sizeof(tuple_count));
}


TupleCollection::~TupleCollection() {
  // TODO: VER FLUSH
  page.make_dirty();
	buffer_manager.unpin(page);
  file_manager.flush(page.page_id, page.get_bytes());
}


bool TupleCollection::is_full() const {
    if (((*tuple_count) + 1) * tuple_size * sizeof(uint64_t) + sizeof(tuple_count) < Page::PAGE_SIZE) {
      return false;
    }
    return true;
}

uint64_t TupleCollection::get_n_tuples() const {
  return *tuple_count;
}


void TupleCollection::add(std::vector<uint64_t> new_tuple) {
	  for (size_t i = 0; i < tuple_size; i++) {
    	tuples[(*tuple_count) * tuple_size + i] = new_tuple[i];
  	}
  	(*tuple_count)++;
}


std::vector<uint64_t> TupleCollection::get(uint_fast64_t id) const {
    std::vector<uint64_t> n_tuple = std::vector<uint64_t>(tuple_size);
    for (size_t i = 0; i < tuple_size; i++) {
        n_tuple[i] = tuples[id * tuple_size + i];
    }
    return n_tuple;
}


void TupleCollection::override_tuple(std::vector<uint64_t>& tuple, int position) {
    for (size_t i = 0; i < tuple_size; i++) {
        tuples[position * tuple_size + i] = tuple[i];
    }
}


void TupleCollection::reset() {
    (*tuple_count) = 0;
}


void TupleCollection::swap(int x, int y) {
    std::vector<uint64_t> x_tuple = get(x);
    std::vector<uint64_t> y_tuple = get(y);
    override_tuple(x_tuple, y);
    override_tuple(y_tuple, x);
}


void TupleCollection::sort( bool (*has_priority)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars),std::vector<uint64_t> order_vars) {
    quicksort(0, (*tuple_count) - 1, has_priority, order_vars);
}


void TupleCollection::quicksort(int i, int f, bool (*has_priority)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars) {
    if (i <= f) {
        int p = partition(i, f, has_priority, order_vars);
        quicksort(i, p - 1, has_priority, order_vars);
        quicksort(p + 1, f, has_priority, order_vars);
    }
}


int TupleCollection::partition(int i, int f, bool (*has_priority)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars) {
    int x = i + (rand() % (f - i + 1));
    std::vector<uint64_t> p = get(x);
    TupleCollection::swap(x,f);
    override_tuple(p, f);
    int j = i;
    int k = i;
    while (k <= f) {
        if (has_priority(get(k), p, order_vars)) {
            TupleCollection::swap(j, k);
        }
        k++;
    }
    TupleCollection::swap(j, f);
    return j;
}

MergeOrderedTupleCollection::MergeOrderedTupleCollection(
    size_t tuple_size,
    std::vector<uint_fast64_t> order_vars,
    bool (*has_priority)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars)) :
        tuple_size(tuple_size),
        order_vars(order_vars),
        has_priority(has_priority),
        left_tuple(std::vector<uint_fast64_t>(tuple_size)),
        right_tuple(std::vector<uint_fast64_t>(tuple_size)) { }


void MergeOrderedTupleCollection::merge(
    uint_fast64_t left_start,
    uint_fast64_t left_end,
    uint_fast64_t right_start,
    uint_fast64_t right_end,
    FileId source_file_id,
    FileId output_file_id
    ) {
    auto left_run = std::make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, left_start), tuple_size);
    auto right_run = std::make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, right_start), tuple_size);
    auto out_run = std::make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, left_start), tuple_size);
    left_tuple = left_run->get(0);
    right_tuple = right_run->get(0);
    uint_fast64_t left_counter = 0;
    uint_fast64_t right_counter = 0;
    uint_fast64_t out_page_counter = left_start;
    bool open_left = true;
    bool open_right = true;
    while (open_left && open_right) {
        if (out_run->is_full()) {
            out_page_counter++;
            out_run = std::make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, out_page_counter), tuple_size);
            out_run->reset();
        }
        if ((has_priority(left_tuple, right_tuple, order_vars) || (right_start > right_end)) && open_left) {
            out_run->add(left_tuple);
            left_counter++;
            if (left_counter == left_run->get_n_tuples()) {
                left_start++;
                if (left_start <= left_end) {
                    left_run = std::make_unique<TupleCollection>(buffer_manager.get_page(source_file_id, left_start), tuple_size);
                    left_counter = 0;
                } else {
                    open_left = false;
                    continue;
                }
            }
            left_tuple = left_run->get(left_counter);
        }
        else if ((has_priority(right_tuple, left_tuple, order_vars) || (left_start > left_end)) && open_right) {
            out_run->add(right_tuple);
            right_counter++;
            if (right_counter == right_run->get_n_tuples()) {
                right_start++;
                if (right_start <= right_end) {
                    right_run = std::make_unique<TupleCollection>(buffer_manager.get_page(output_file_id, right_start), tuple_size);
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
          std::vector<uint64_t> t = source_tuples->get(i);
          uint64_t a = t[0];
          output_tuples->add(move(t));
          uint64_t b = output_tuples->get(i)[0];
          if (a != b) {
            cout << "algo malo hay\n";
          }
      }
      //source_tuples->reset();
}
