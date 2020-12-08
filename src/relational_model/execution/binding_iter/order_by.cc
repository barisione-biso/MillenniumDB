#include "order_by.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>


#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/tuple_collection/tuple_collection.h"


bool is_lt(std::vector<uint64_t> a, std::vector<uint64_t> b, std::vector<uint64_t> order_vars) {
    for (size_t i = 0; i < order_vars.size(); i++) {
        if (a[i] < b[i]) {
            return true;
        }
        if (b[i] < a[i]) {
            return false;
        }
    }
    return true;
}

bool is_gt(std::vector<uint64_t> a, std::vector<uint64_t> b, std::vector<uint64_t> order_vars) {
    for (size_t i = 0; i < order_vars.size(); i++) {
        if (a[i] > b[i]) {
            return true;
        }
        if (b[i] > a[i]) {
            return false;
        }
    }
    return true;
}

using namespace std;

OrderBy::OrderBy(GraphModel& model,
                 std::unique_ptr<BindingIter> _child,
                 vector<pair<string, VarId>> order_vars,
                 size_t binding_size,
                 const bool ascending) :
    child        (move(_child)),
    order_vars   (move(order_vars)),
    binding_size (binding_size),
    my_binding   (BindingOrderBy(model, move(order_vars), child->get_binding(), binding_size)),
    first_file_id     (file_manager.get_file_id("temp0.txt")),
    second_file_id    (file_manager.get_file_id("temp1.txt"))
{
    bool (*has_priority)(std::vector<uint64_t> a, std::vector<uint64_t>b, std::vector<uint64_t> order_v) = (ascending) ? is_lt : is_gt;
    std::vector<uint64_t> order_ids = std::vector<uint64_t>(order_vars.size());
    for (size_t i = 0; i < order_vars.size(); i++) {
        order_ids[i] = order_vars[i].second.id;
    }
    n_pages = 0;
    merger = make_unique<MergeOrderedTupleCollection>(binding_size, order_ids, has_priority);
    run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, n_pages), binding_size);
    run->reset();
    std::vector<uint64_t> binding_id_vec = std::vector<uint64_t>(binding_size);
    while (child->next()) {
        if (run->is_full()) {
            n_pages++;
            run->sort(has_priority, order_ids);
            run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, n_pages), binding_size);
            run->reset();
        }
        for (size_t i = 0; i < binding_size; i++) {
            binding_id_vec[i] = my_binding.get_id(VarId(i)).id;
        }
        run->add(binding_id_vec);
    }
    run = nullptr;
    file_id_n = mergeSort(0, n_pages, -1);
    run = make_unique<TupleCollection>(buffer_manager.get_page(file_id_n == 2 ? second_file_id : first_file_id, 0), binding_size);
}

OrderBy::~OrderBy() {
    //buffer_manager.remove(first_file_id);
    //buffer_manager.remove(second_file_id);
}

Binding& OrderBy::get_binding() {
    return my_binding;
}


bool OrderBy::next() {
    if (page_position == run->get_n_tuples()) {
        current_page++;
        if (current_page > n_pages) {
            return false;
        }
        run = make_unique<TupleCollection>(buffer_manager.get_page(file_id_n == 2 ? second_file_id : first_file_id, current_page), binding_size);
        page_position = 0;
    }
    std::vector<uint64_t> binding_ids = run->get(page_position);
    my_binding.update_binding_object(move(binding_ids));
    page_position++;
    return true;
}

void OrderBy::analyze(int indent) const {
    child->analyze(indent);
}

int OrderBy::mergeSort(uint_fast64_t start_page, uint_fast64_t end_page, int file_n) {
    // false -> output save in first, output saved in second
    if (start_page == end_page) {
      if (start_page == 0) {
        return 1;
      }
      if (file_n == 2) {
        merger->copy_page(start_page, first_file_id, second_file_id);
        return 2;
      }
      return 1;
    }
    uint_fast64_t middle = (start_page + end_page) / 2;
    int file_n_save = mergeSort(start_page, middle, file_n);
    mergeSort(middle + 1, end_page, file_n == -1 ? file_n_save : file_n);
    int output_file_n = file_n == -1 ? file_n_save == 2 ? 1 : 2 : file_n;
    if (output_file_n == 1) {
        merger->merge(
          start_page,
          middle,
          middle + 1,
          end_page,
          second_file_id,
          first_file_id);
    } else {
        merger->merge(
          start_page,
          middle,
          middle + 1,
          end_page,
          first_file_id,
          second_file_id);
    }
    return 2;
}

