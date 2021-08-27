#include "order_by.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>

#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "storage/tuple_collection/tuple_collection.h"

using namespace std;

OrderBy::OrderBy(const GraphModel& model,
                 ThreadInfo* thread_info,
                 unique_ptr<BindingIter> child,
                 size_t binding_size,
                 vector<pair<Var, VarId>> order_vars,
                 vector<bool> ascending) :
    thread_info    (thread_info),
    child          (move(child)),
    order_vars     (move(order_vars)),
    ascending      (move(ascending)),
    binding_size   (binding_size),
    my_binding     (BindingOrderBy(model, binding_size)),
    first_file_id  (file_manager.get_tmp_file_id()),
    second_file_id (file_manager.get_tmp_file_id()) { }


OrderBy::~OrderBy() {
    run.reset();
    file_manager.remove_tmp(first_file_id);
    file_manager.remove_tmp(second_file_id);
}


void OrderBy::begin() {
    child->begin();
    std::vector<VarId> order_var_ids;
    for (const auto& order_var : order_vars) {
        order_var_ids.push_back(order_var.second);
    }

    total_pages = 0;
    run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(first_file_id, total_pages), binding_size);
    run->reset();
    std::vector<GraphObject> graph_objects(binding_size);
    auto& child_binding = child->get_binding();
    // Save all the tuples of child in disk and apply sort to each page
    while (child->next()) {
        if (run->is_full()) {
            total_pages++;
            run->sort(order_var_ids, ascending);
            run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(first_file_id, total_pages), binding_size);
            run->reset();
        }
        for (size_t i = 0; i < binding_size; i++) {
            GraphObject graph_obj = child_binding[VarId(i)];
            graph_objects[i] = graph_obj;
        }
        run->add(graph_objects);
    }
    run->sort(order_var_ids, ascending);
    total_pages++;
    run = nullptr;
    merge_sort(order_var_ids);
    run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(*output_file_id, 0), binding_size);
}


bool OrderBy::next() {
    if (page_position == run->get_tuple_count()) {
        current_page++;
        if (current_page >= total_pages) {
            return false;
        }
        run = make_unique<TupleCollection>(buffer_manager.get_tmp_page(*output_file_id, current_page), binding_size);
        page_position = 0;
    }
    auto graph_object = run->get(page_position);
    my_binding.update_binding_object(graph_object);
    page_position++;
    return true;
}


void OrderBy::analyze(std::ostream& os, int indent) const {
    child->analyze(os, indent);
}


void OrderBy::merge_sort(const std::vector<VarId>& order_var_ids) {
    // Iterative merge sort implementation. Run to merge are a power of two
    uint_fast64_t start_page;
    uint_fast64_t end_page;
    uint_fast64_t middle;
    uint_fast64_t runs_to_merge = 1;

    MergeOrderedTupleCollection merger(binding_size, order_var_ids, ascending, &thread_info->interruption_requested);

    // output_file_id = &first_file_id;
    bool output_is_in_second = false;

    TmpFileId* source_pointer = &first_file_id;
    TmpFileId* dest_pointer   = &second_file_id;

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
                merger.copy_page(start_page, *source_pointer, *dest_pointer);
            } else {
                merger.merge(start_page, middle, middle + 1, end_page, *source_pointer, *dest_pointer);
            }
            start_page = end_page + 1;
            middle = start_page + (runs_to_merge / 2) - 1;
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
