#include "group_by.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/tuple_collection/tuple_collection.h"

using namespace std;

GroupBy::GroupBy(GraphModel& model,
                 std::unique_ptr<BindingIter> _child,
                 size_t _binding_size,
                 vector<pair<string, VarId>> _group_vars,
                 vector<bool> ascending) :
    order_child    (OrderBy(model, move(_child), _binding_size, _group_vars, ascending)),
    binding_size   (_binding_size),
    group_vars     (_group_vars),
    my_binding     (BindingGroupBy(model, group_vars, order_child.get_binding(), _binding_size)),
    group_file_id  (file_manager.get_file_id("group_file.txt")) // TODO:
{
    auto& child_binding = order_child.get_binding();
    group_run = make_unique<TupleCollection>(buffer_manager.get_page(group_file_id, n_pages), binding_size);
    group_run->reset();
    current_group_tuple = std::vector<GraphObject>(binding_size);
    current_tuple = std::vector<GraphObject>(binding_size);

    if (order_child.next()) {
        for (size_t i = 0; i < binding_size; i++) {
            GraphObject graph_obj = child_binding[VarId(i)];
            current_group_tuple[i] = graph_obj;
            current_tuple[i] = graph_obj;
        }
        add_tuple_to_group();
    }
}


Binding& GroupBy::get_binding() {
    return my_binding;
}


bool GroupBy::next() {
    auto& child_binding = order_child.get_binding();
    current_tuple = std::vector<GraphObject>(binding_size);
    while (order_child.next()) {
        for (size_t i = 0; i < binding_size; i++) {
            GraphObject graph_obj = child_binding[VarId(i)];
            current_tuple[i] = graph_obj;
        }
        if (has_same_group_vars()) {
            add_tuple_to_group();
        } else {
            compute_agregation_function();
            n_pages = 0;
            group_run = make_unique<TupleCollection>(buffer_manager.get_page(group_file_id, n_pages), binding_size);
            group_run->reset();
            add_tuple_to_group();
            current_group_tuple = current_tuple;
            return true;
         }
    }
    if (group_run->get_tuple_count()) {
        compute_agregation_function();
        n_pages = 0;
        group_run = make_unique<TupleCollection>(buffer_manager.get_page(group_file_id, 0), binding_size);
        group_run->reset();
          return true;
    }
    return false;
}


void GroupBy::analyze(int indent) const {
    order_child.analyze(indent);
}


bool GroupBy::has_same_group_vars() {
    for (size_t i = 0; i < group_vars.size(); i++) {
        if (current_group_tuple[group_vars[i].second.id] != current_tuple[group_vars[i].second.id]) {
          return false;
        }
    }
    return true;
}


void GroupBy::add_tuple_to_group() {
    if (group_run->is_full()) {
        n_pages++;
        group_run = make_unique<TupleCollection>(buffer_manager.get_page(group_file_id, n_pages), binding_size);
        group_run->reset();
    }
    group_run->add(current_tuple);
}


// TODO: Agregation function. Now only return the first tuple of the group
void GroupBy::compute_agregation_function() {
    group_run = make_unique<TupleCollection>(buffer_manager.get_page(group_file_id, 0), binding_size);
    my_binding.update_binding(group_run->get(0));
}