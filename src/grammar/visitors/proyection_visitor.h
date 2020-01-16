#ifndef VISITORS__PROYECTION_VISITOR_H_
#define VISITORS__PROYECTION_VISITOR_H_

#include "grammar/ast.h"

#include <map>
#include <string>
#include <vector>
#include <boost/variant.hpp>

namespace visitors {
    class ProyectionVisitor : public boost::static_visitor<void> {

    private:
        std::vector<std::string> names;
        std::vector<int_fast32_t> var_positions;
        std::map<std::string, uint_fast32_t> id_map;
        std::map<std::string, std::string> constants;
    public:
        ProyectionVisitor(std::map<std::string, uint_fast32_t> id_map, std::map<std::string, std::string> constants)
            : id_map(std::move(id_map)), constants(std::move(constants))
        { }
        ~ProyectionVisitor() = default;

        std::vector<std::string> get_names() {
            return names;
        }

        std::vector<int_fast32_t> get_var_positions() {
            return var_positions;
        }

        void operator()(ast::root const& r) {
            boost::apply_visitor(*this, r.selection_);
        }

        void operator()(ast::all_ const&) {
            // TODO: not suported
        }

        void operator()(std::vector<ast::element> const& elements) {
            for (auto& element : elements) {
                std::string var_name = element.variable_ + "." + element.key_;
                names.push_back(var_name);
                if (constants.find(var_name) == constants.end()) {
                    var_positions.push_back(id_map[var_name]);
                }
                else {
                    var_positions.push_back(-1);
                }
            }
        }
    };

}; // namespace visitors


#endif // VISITORS__PROYECTION_VISITOR_H_