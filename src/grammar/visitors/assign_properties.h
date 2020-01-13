#ifndef GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H
#define GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::map<std::string, std::map<std::string, uint_fast32_t>> str_str_int_map;
    typedef std::map<uint_fast32_t, std::map<std::string, ast::value>> int_str_value_map;
    typedef std::map<std::string, uint_fast32_t> str_int_map;
#endif

namespace visitors {
    // Auxiliary visitor to get the correspoding index to boost::variant
    // according to value
    class whichVisitor
        : public boost::static_visitor<uint32_t> {
    public:
        static const uint32_t NOT_VALUE = UINT32_MAX;

        int operator()(ast::value const& val) const { return val.which(); }
        int operator()(ast::element const&) const { return UINT32_MAX; }
    }; // class whichVisitor

    // Fourth visitor retrieves the properties asociated
    // to each VarID and checks consistency in the type
    // of values at WHERE and MATCH statements. Returns
    // a VarID to map of Key Values map.
    class asssignProperties
        : public boost::static_visitor<void>
    {
        str_int_map id_map;
        int_str_value_map property_map;
        str_str_int_map seen_value_type;

        public:

        // Constructor
        asssignProperties(str_int_map & id_map)
            : id_map(id_map) { }

        int_str_value_map get_property_map() {
            return property_map;
        }

        void operator()(ast::root const& r) {
            for(auto const& linear_pattern: r.graphPattern_) {
                (*this)(linear_pattern);
            }
            (*this)(r.where_);
            boost::apply_visitor(*this, r.selection_);
        }

        void operator()(ast::linear_pattern const& linear_pattern) {
            (*this)(linear_pattern.root_);
            for(auto &step_path: linear_pattern.path_) {
                (*this)(step_path.edge_);
                (*this)(step_path.node_);
            }
        }

        void operator()(ast::edge const& edge) {
            for (auto& prop : edge.properties_) {
                property_map[id_map.at(edge.variable_)][prop.key_] = prop.value_;
            }
        }

        void operator()(ast::node const& node) {
            for (auto& prop : node.properties_) {
                property_map[id_map.at(node.variable_)][prop.key_] = prop.value_;
            }
        }

        // Wrapper for the case of the optional where statement in ast::root
        void operator()(boost::optional<ast::formula> const& formula) {
            if (formula) {
                ast::formula real_formula = static_cast<ast::formula>(formula.get());
                boost::apply_visitor(*this, real_formula.root_);
                for (auto& step_formula : real_formula.path_) {
                    // TODO: assuming step_formula.op_ is AND
                    boost::apply_visitor(*this, step_formula.cond_);
                }
            }
        }


        void operator()(ast::formula const& formula) {
            boost::apply_visitor(*this, formula.root_);
            for (auto& step_formula: formula.path_) {
                // TODO: assuming step_formula.op_ is AND
                boost::apply_visitor(*this, step_formula.cond_);
            }
        }


        void operator()(ast::statement const& stat) {

            // Check consistencies
            auto storedWhich = boost::apply_visitor(whichVisitor(), stat.rhs_);
            if (storedWhich != whichVisitor::NOT_VALUE) { // Check type consistency against declared properties
                uint_fast32_t id_ = id_map.at(stat.lhs_.variable_);
                auto entMap = property_map.find(id_);
                if (entMap != property_map.end()) {
                    auto foundIt = entMap->second.find(stat.lhs_.key_);
                    if (foundIt != entMap->second.end()) {
                        if (storedWhich != (uint_fast32_t) foundIt->second.which()) {
                            throw ast::TypeError(stat.lhs_.variable_);
                        }
                    }
                }
                else { // Check type consistency against other statements
                    auto foundMap = seen_value_type.find(stat.lhs_.variable_);
                    if (foundMap != seen_value_type.end()) {
                        auto foundIt = foundMap->second.find(stat.lhs_.key_);
                        if (foundIt != foundMap->second.end()) {
                            if (storedWhich != foundIt->second) {
                                throw ast::TypeError(stat.lhs_.variable_);
                            }
                        }
                        else {
                            foundMap->second[stat.lhs_.key_] = storedWhich;
                        }
                    }
                    else {
                        seen_value_type[stat.lhs_.variable_][stat.lhs_.key_] = storedWhich;
                    }
                }
            }

            // TODO: assuming statemens are in disyuntive normal form and only equals is supported
            if (stat.comparator_.which() != 0) {
                throw ast::NotSupportedError("Only equals condition is supported");
            }
            // TODO: maybe a another visitor is better
            // TODO: for now assuming conditions like ?n.name == "John"
            // uint_fast32_t lhs_aux_var_id = id_map.at(stat.lhs_.variable_ + "." + stat.lhs_.key_);
            if (stat.rhs_.which() == 1) { // condition like ?var.key == literal
                uint_fast32_t lhs_var_id = id_map.at(stat.lhs_.variable_);
                ast::value rhs_value = boost::get<ast::value>(stat.rhs_);
                property_map[lhs_var_id][stat.lhs_.key_] = rhs_value;
            }
        }


        void operator()(std::vector<ast::element> const& element_list) {
            for (auto const& element : element_list) {
                uint_fast32_t var_id = id_map.at(element.variable_);
                auto var_found = property_map.find(var_id);
                if (var_found != property_map.end()) {
                    auto foundKey = var_found->second.find(element.key_);
                        if (foundKey != var_found->second.end())
                            continue;
                }
                uint_fast32_t aux_var_id = id_map.at(element.variable_ + "." + element.key_);
                property_map[var_id][element.key_] = VarId(aux_var_id);
            }
        }

        void operator()(ast::all_ const&) {}

    }; // class assignProperties

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H
