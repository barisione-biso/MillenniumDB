#include "binding_iter_visitor.h"

#include <iostream>

#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/visitors/formula_to_condition.h"
#include "relational_model/execution/binding_id_iter/optional_node.h"
#include "relational_model/execution/binding_iter/match.h"
#include "relational_model/execution/binding_iter/select.h"
#include "relational_model/execution/binding_iter/where.h"
#include "relational_model/models/quad_model/query_optimizer/binding_id_iter_visitor.h"

using namespace std;

BindingIterVisitor::BindingIterVisitor(QuadModel& model, std::set<std::string> var_names) :
    model           (model),
    var_name2var_id (construct_var_name2var_id(var_names)) { }


map<string, VarId> BindingIterVisitor::construct_var_name2var_id(std::set<std::string>& var_names) {
    map<string, VarId> res;
    uint_fast32_t i = 0;
    cout << "printing var names:\n";
    for (auto& var_name : var_names) {
        cout << var_name << "\n";
        res.insert({ var_name, VarId(i++) });
    }
    return res;
}


// TODO: ScanRange::get
std::unique_ptr<ScanRange> BindingIterVisitor::get_scan_range(Id id, bool assigned) {
    if ( std::holds_alternative<ObjectId>(id) ) {
        return std::make_unique<Term>(std::get<ObjectId>(id));
    } else if (assigned) {
        return std::make_unique<AssignedVar>(std::get<VarId>(id));
    } else {
        return std::make_unique<UnassignedVar>(std::get<VarId>(id));
    }
}


std::unique_ptr<BindingIter> BindingIterVisitor::exec(OpSelect& op_select) {
    op_select.accept_visitor(*this);
    return move(tmp);
}


std::unique_ptr<BindingIter> BindingIterVisitor::exec(manual_plan::ast::ManualRoot&) {
    // TODO:
    return NULL;
}


void BindingIterVisitor::visit(const OpSelect& op_select) {
    // need to save the select items to be able to push optional properties from select to match in visit(OpGraphPatternRoot&)
    select_items = move(op_select.select_items);

    // TODO: set var_name2var_id
    op_select.op->accept_visitor(*this);

    vector<pair<string, VarId>> projection_vars;
    
    if (select_items.size() == 0) { // SELECT *
        for (auto&& [k, v] : var_name2var_id) {
            projection_vars.push_back(make_pair(k, v));
        }
    } else {
        for (const auto& select_item : select_items) {
            string var_name = select_item.var; // var_name = "?x"
            if (select_item.key) {
                var_name += '.';
                var_name += select_item.key.get(); // var_name = "?x.key1" MATCH (?x)->(?y)
            }
            // I can supose var_name is always present in var_name2var_id at this point
            //auto var_id = var_name2var_id[var_name]; //TODO: Fix this
            auto var_id = get_var_id(var_name);
            projection_vars.push_back(make_pair(var_name, var_id));
        }
    }
    tmp = make_unique<Select>(move(tmp), move(projection_vars), op_select.limit);
}


void BindingIterVisitor::visit(const OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    auto match_binding_size = var_name2var_id.size(); // var_name2var_id is suposed to be setted in visit(OpGraphPatternRoot&)

    Formula2ConditionVisitor visitor(model, var_name2var_id);
    auto condition = visitor(op_filter.formula);
    auto new_property_var_id = move(visitor.property_map);

    tmp = make_unique<Where>(
        model,
        move(tmp),
        move(condition),
        match_binding_size,
        move(new_property_var_id)
    );
}


void BindingIterVisitor::visit(const OpGraphPatternRoot& op_graph_pattern_root) {

    op_graph_pattern_root.op->get_var_names();

    BindingIdIterVisitor id_visitor(model, var_name2var_id);
    op_graph_pattern_root.op->accept_visitor(id_visitor);

    // var_name2var_id = move(id_visitor.var_name2var_id);
    unique_ptr<BindingIdIter> binding_id_iter_current_root = move(id_visitor.tmp);

    // We need to get the final binding_size
    // `var_name2var_id` may not contain all properties from SELECT, so we call `get_var_id(var_name)`
    // in order add them if they are not present. E.g:
    // SELECT ?x.key
    // MATCH (?x)
    // "?x.key" is not present in `var_name2var_id`, but it will be present after calling `get_var_id("?x.key")`
    /*for (const auto& select_item : select_items) {
        if (select_item.key) {
            get_var_id(select_item.var + '.' + select_item.key.get());
        }
    }*/

    auto binding_size = var_name2var_id.size();
    cout << "binding size:" << binding_size << "\n";
    vector<unique_ptr<BindingIdIter>> optional_children;

    // Push properties from SELECT into MATCH as optional children
    for (const auto& select_item : select_items) {
        if (select_item.key) {
            auto obj_var_id = get_var_id(select_item.var);
            auto value_var  = get_var_id(select_item.var + '.' + select_item.key.get());
            auto key_id     = model.get_string_id(select_item.key.get());

            array<unique_ptr<ScanRange>, 3> ranges {
                get_scan_range(obj_var_id, true),
                get_scan_range(key_id, true),
                get_scan_range(value_var, false)
            };
            auto index_scan = make_unique<IndexScan<3>>(binding_size, *model.object_key_value, move(ranges));
            optional_children.push_back(move(index_scan));
        }
    }
    if (optional_children.size() > 0) {
        binding_id_iter_current_root = make_unique<OptionalNode>(binding_size, move(binding_id_iter_current_root), move(optional_children));
    }

    tmp = make_unique<Match>(model, move(binding_id_iter_current_root), binding_size);
    cout << "Finished BindingIterVisitor\n";
}


void BindingIterVisitor::visit(const OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}


void BindingIterVisitor::visit(const OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);
}


ObjectId BindingIterVisitor::get_value_id(const common::ast::Value& value) {
    if (value.type() == typeid(string)) {
        return model.get_object_id(GraphObject::make_string( boost::get<string>(value) ));
    }
    else if (value.type() == typeid(int64_t)) {
        return model.get_object_id(GraphObject::make_int( boost::get<int64_t>(value) ));
    }
    else if (value.type() == typeid(float)) {
        return model.get_object_id(GraphObject::make_float( boost::get<float>(value) ));
    }
    else if (value.type() == typeid(bool)) {
        return model.get_object_id(GraphObject::make_bool( boost::get<bool>(value) ));
    }
    else {
        throw logic_error("Unknown value type.");
    }
}


// You only should use this after var_name2var_id was setted at visit(OpGraphPatternRoot)
VarId BindingIterVisitor::get_var_id(const std::string& var) {
    auto search = var_name2var_id.find(var);
    if (search != var_name2var_id.end()) {
        return (*search).second;
    } else {
        throw std::logic_error("variable " + var + " not present in var_name2var_id");
        // VarId res(var_name2var_id.size());
        // var_name2var_id.insert({ var, res });
        // return res;
    }
}

void BindingIterVisitor::visit(const OpMatch&) { }
void BindingIterVisitor::visit(const OpOptional&) { }
void BindingIterVisitor::visit(const OpConnection&) { }
void BindingIterVisitor::visit(const OpTransitiveClosure&) { }
void BindingIterVisitor::visit(const OpUnjointObject&) { }
void BindingIterVisitor::visit(const OpConnectionType&) { }
void BindingIterVisitor::visit(const OpLabel&) { }
void BindingIterVisitor::visit(const OpProperty&) { }
