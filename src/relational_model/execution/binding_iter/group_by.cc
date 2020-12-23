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
                 vector<pair<string, VarId>> group_vars,
                 size_t _binding_size
                 ) :
    order_child    (OrderBy(model, move(_child), group_vars, binding_size, true)),
    binding_size   (_binding_size),
    my_binding     (BindingGroupBy(model, move(group_vars), order_child.get_binding(), binding_size)),
    group_file_id (file_manager.get_file_id("group_file.txt"))
{ }


Binding& GroupBy::get_binding() {
    return my_binding;
}


bool GroupBy::next() {
   if (order_child.next()) {
     return true;
   }
   return false;
}

void GroupBy::analyze(int indent) const {
    order_child.analyze(indent);
}
