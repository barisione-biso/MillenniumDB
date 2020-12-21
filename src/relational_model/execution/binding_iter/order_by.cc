#include "order_by.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>


#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/tuple_collection/tuple_collection.h"

using namespace std;


OrderBy::OrderBy(GraphModel& model,
                 std::unique_ptr<BindingIter> _child,
                 vector<pair<string, VarId>> order_vars,
                 size_t binding_size,
                 const bool ascending) :
    child          (move(_child)),
    binding_size   (binding_size),
    my_binding     (BindingOrderBy(model, move(order_vars), child->get_binding(), binding_size)),
    first_file_id  (file_manager.get_file_id("temp0.txt")),
    second_file_id (file_manager.get_file_id("temp1.txt"))
{
    std::vector<uint_fast64_t> order_ids = std::vector<uint64_t>(my_binding.order_vars.size());
    for (size_t i = 0; i < my_binding.order_vars.size(); i++) {
        order_ids[i] = my_binding.order_vars[i].second.id;
    }
    n_pages = 0;
    merger = make_unique<MergeOrderedTupleCollection>(binding_size, order_ids, ascending);
    run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, n_pages), binding_size);
    run->reset();
    std::vector<GraphObject> graph_objects(binding_size);
    while (child->next()) {
        if (run->is_full()) {
            n_pages++;
            run->sort(order_ids, ascending);
            run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, n_pages), binding_size);
            run->reset();
        }
        for (size_t i = 0; i < binding_size; i++) {
            GraphObject graph_obj = my_binding[VarId(i)];
            graph_objects[i] = graph_obj;
        }
        run->add(graph_objects);
    }
    run->sort(order_ids, ascending); // TODO: Revisar este ultimo sort
    n_pages++;
    my_binding.finish_read_of_child();
    run = nullptr;
    output_file_id = &first_file_id;
    mergeSort();
    run = make_unique<TupleCollection>(buffer_manager.get_page(*output_file_id, 0), binding_size);
}


Binding& OrderBy::get_binding() {
    return my_binding;
}


bool OrderBy::next() {
    if (page_position == run->get_n_tuples()) {
        current_page++;
        if (current_page == n_pages) {
            return false;
        }
        run = make_unique<TupleCollection>(buffer_manager.get_page(*output_file_id, current_page), binding_size);
        page_position = 0;
    }
    auto graph_object = run->get(page_position);
    my_binding.update_binding_object(graph_object);
    page_position++;
    return true;
}

void OrderBy::analyze(int indent) const {
    child->analyze(indent);
}


void OrderBy::mergeSort() {
    uint_fast64_t start_page;
    uint_fast64_t end_page;
    uint_fast64_t middle;
    uint_fast64_t runs_to_merge = 1;
    bool output_is_in_second = false;
    FileId* source_pointer = &first_file_id;
    FileId* output_pointer = &second_file_id;
    while (runs_to_merge < n_pages) {
        runs_to_merge *= 2;
        start_page = 0;
        middle = (runs_to_merge / 2) - 1;
        if (runs_to_merge > n_pages) {
            runs_to_merge = n_pages;
            end_page = runs_to_merge - 1;
        } else {
            end_page = runs_to_merge - 1;
        }
        while (start_page < n_pages) {
            if (start_page == end_page) {
                merger->copy_page(start_page, *source_pointer, *output_pointer);
            } else {
                merger->merge(start_page, middle, middle + 1, end_page, *source_pointer, *output_pointer);
            }
            start_page = end_page + 1;
            middle = start_page +  (runs_to_merge / 2)  - 1;
            end_page += runs_to_merge;
            if (end_page >= n_pages) {
                end_page = n_pages - 1;
            }
            if (middle >= end_page) {
                middle = (start_page + end_page) / 2;
            }
        }
        output_is_in_second = !output_is_in_second;
        source_pointer = output_is_in_second ? &second_file_id : &first_file_id;
        output_pointer = output_is_in_second ? &first_file_id : &second_file_id;
    }
    output_file_id = output_is_in_second ? &second_file_id : &first_file_id;
}
