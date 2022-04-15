#include "return_item_visitor_impl.h"

#include "execution/binding_iter/aggregation/aggs.h"
#include "parser/query/return_item/return_items.h"

void ReturnItemVisitorImpl::visit(ReturnItemAgg& /*return_item*/) {
    // TODO: implement when supporting aggregate functions
}


void ReturnItemVisitorImpl::visit(ReturnItemCount& return_item) {
    if (return_item.inside_var == "*") {
        Var var = return_item.get_var();
        auto var_id = binding_iter_visitor.get_var_id(var);
        if (return_item.distinct) {
            std::vector<VarId> var_ids;
            for (auto&& [var, var_id] : binding_iter_visitor.var2var_id) {
                if (var.name[0] == '?' && var.name[1] != '_') { // TODO: should be done more elegant
                    var_ids.push_back(var_id);
                }
            }

            aggregates.insert({var_id , std::make_unique<AggCountAllDistinct>(std::move(var_ids))});
        } else {
            aggregates.insert({var_id , std::make_unique<AggCountAll>()});
        }
        projection_vars.push_back({ var , var_id });
    } else {
        auto pos = return_item.inside_var.find('.');
        if (pos != std::string::npos) {
            // we split something like "?x1.key1" into "?x" and "key1"
            auto var_without_property = return_item.inside_var.substr(0, pos);
            auto var_key              = return_item.inside_var.substr(pos + 1);
            var_properties.insert({ Var(var_without_property), var_key });
        }
        Var var = return_item.get_var();
        auto var_id = binding_iter_visitor.get_var_id(var);

        Var inside_var(return_item.inside_var);
        auto inside_var_id = binding_iter_visitor.get_var_id(inside_var);
        if (return_item.distinct) {
            aggregates.insert({var_id , std::make_unique<AggCountVarDistinct>(inside_var_id)});
        } else {
            aggregates.insert({var_id , std::make_unique<AggCountVar>(inside_var_id)});
        }
        projection_vars.push_back({ var , var_id });
    }
}


void ReturnItemVisitorImpl::visit(ReturnItemVar& return_item) {
    std::string& var_str = return_item.var.name;
    auto pos = var_str.find('.');
    if (pos != std::string::npos) {
        // we split something like "?x1.key1" into "?x" and "key1"
        auto var_without_property = var_str.substr(0, pos);
        auto var_key              = var_str.substr(pos + 1);
        var_properties.insert({ Var(var_without_property), var_key });
    }

    Var var(var_str);
    auto var_id = binding_iter_visitor.get_var_id(var);
    projection_vars.push_back({ var, var_id });
}
