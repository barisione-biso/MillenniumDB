#include "return_item_check_var_name.h"

#include "base/exceptions.h"
#include "parser/query/return_item/return_items.h"

void ReturnItemCheckVarName::visit(ReturnItemAgg& return_item) {
    // TODO: implement when supporting aggregate functions

}


void ReturnItemCheckVarName::visit(ReturnItemCount& return_item) {
    if (return_item.inside_var == "*") {
        return;
    }

    auto& var_name = return_item.inside_var;
    Var var(var_name);

    auto pos = var_name.find('.');
    if (pos != std::string::npos) {
        // var is like "?x.key1" transform into "?x"
        Var var_without_property(var_name.substr(0, pos));
        if (declared_vars.find(var_without_property) == declared_vars.end()) {
            throw QuerySemanticException("Variable \"" + var_without_property.name
                                        + "\" used in RETURN (property) is not declared in MATCH");
        } else if (declared_path_vars.find(var_without_property) != declared_path_vars.end()) {
            throw QuerySemanticException("Variable \"" + var_without_property.name
                                        + "\" is a path and cannot have properties");
        }
    } else {
        // var is like "?x"
        if (declared_vars.find(var) == declared_vars.end()) {
            throw QuerySemanticException("Variable \"" + var_name + "\" used in RETURN is not declared in MATCH");
        }
    }
    if (op_return_distinct) {
        if (declared_path_vars.find(var) != declared_path_vars.end()) {
            throw QuerySemanticException("DISTINCT of path variable \"" + var.name
                                        + "\" is not supported yet");
        }
    }
}


void ReturnItemCheckVarName::visit(ReturnItemVar& return_item) {
    Var var = return_item.get_var();
    auto pos = var.name.find('.');
    if (pos != std::string::npos) {
        // var is like "?x.key1" transform into "?x"
        Var var_without_property(var.name.substr(0, pos));
        if (declared_vars.find(var_without_property) == declared_vars.end()) {
            throw QuerySemanticException("Variable \"" + var_without_property.name
                                        + "\" used in RETURN (property) is not declared in MATCH");
        } else if (declared_path_vars.find(var_without_property) != declared_path_vars.end()) {
            throw QuerySemanticException("Variable \"" + var_without_property.name
                                        + "\" is a path and cannot have properties");
        }
    } else {
        // var is like "?x"
        if (declared_vars.find(var) == declared_vars.end()) {
            throw QuerySemanticException("Variable \"" + var.name + "\" used in RETURN is not declared in MATCH");
        }
    }
    if (op_return_distinct) {
        if (declared_path_vars.find(var) != declared_path_vars.end()) {
            throw QuerySemanticException("DISTINCT of path variable \"" + var.name
                                        + "\" is not supported yet");
        }
    }
}
