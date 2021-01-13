#include "order_by.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>

#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/tuple_collection/tuple_collection.h"

using namespace std;

OrderBy::OrderBy(GraphModel& model,
                 unique_ptr<BindingIter> _child,
                 size_t binding_size,
                 vector<pair<string, VarId>> order_vars,
                 vector<bool> ascending) :
    child          (move(_child)),
    binding_size   (binding_size),
    my_binding     (BindingOrderBy(model, binding_size)),
    first_file_id  (file_manager.get_file_id("temp0.txt")),
    second_file_id (file_manager.get_file_id("temp1.txt"))
    // first_file_id  (file_manager.get_tmp_file_id(query_id)), // TODO:
    // second_file_id (file_manager.get_tmp_file_id(query_id))  // TODO:
{
    std::vector<uint64_t> order_ids(order_vars.size());
    for (size_t i = 0; i < order_vars.size(); i++) {
        order_ids[i] = order_vars[i].second.id;
    }

    total_pages = 0;
    merger = make_unique<MergeOrderedTupleCollection>(binding_size, order_ids, ascending);
    run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, total_pages), binding_size);
    run->reset();
    std::vector<GraphObject> graph_objects(binding_size);
    auto& child_binding = child->get_binding();
    // Save all the tuples of child in disk and apply sort to each page
    while (child->next()) {
        if (run->is_full()) {
            total_pages++;
            run->sort(order_ids, ascending);
            run = make_unique<TupleCollection>(buffer_manager.get_page(first_file_id, total_pages), binding_size);
            run->reset();
        }
        for (size_t i = 0; i < binding_size; i++) {
            GraphObject graph_obj = child_binding[VarId(i)];
            graph_objects[i] = graph_obj;
        }
        run->add(graph_objects);
    }
    run->sort(order_ids, ascending);
    total_pages++;
    run = nullptr;
    merge_sort();
    run = make_unique<TupleCollection>(buffer_manager.get_page(*output_file_id, 0), binding_size);
}


Binding& OrderBy::get_binding() {
    return my_binding;
}


bool OrderBy::next() {
    if (page_position == run->get_tuple_count()) {
        current_page++;
        if (current_page >= total_pages) {
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


void OrderBy::merge_sort() {
    // Iterative merge sort implementation. Run to merge are a power of two
    uint_fast64_t start_page;
    uint_fast64_t end_page;
    uint_fast64_t middle;
    uint_fast64_t runs_to_merge = 1;

    // output_file_id = &first_file_id;
    bool output_is_in_second = false;

    FileId* source_pointer = &first_file_id;
    FileId* dest_pointer   = &second_file_id;

    while (runs_to_merge < total_pages) {
        runs_to_merge *= 2;
        start_page = 0;
        middle = (runs_to_merge / 2) - 1;
        if (runs_to_merge > total_pages) {
            runs_to_merge = total_pages;
            end_page = runs_to_merge - 1;
        } else {
            end_page = runs_to_merge - 1;
        }
        while (start_page < total_pages) {
            if (start_page == end_page) {
                merger->copy_page(start_page, *source_pointer, *dest_pointer);
            } else {
                merger->merge(start_page, middle, middle + 1, end_page, *source_pointer, *dest_pointer);
            }
            start_page = end_page + 1;
            middle = start_page +  (runs_to_merge / 2)  - 1;
            end_page += runs_to_merge;
            if (end_page >= total_pages) {
                end_page = total_pages - 1;
            }
            if (middle >= end_page) {
                middle = (start_page + end_page) / 2;
            }
        }
        output_is_in_second = !output_is_in_second;
        source_pointer = output_is_in_second ? &second_file_id : &first_file_id;
        dest_pointer   = output_is_in_second ? &first_file_id  : &second_file_id;
    }
    output_file_id = output_is_in_second ? &second_file_id : &first_file_id;
}
