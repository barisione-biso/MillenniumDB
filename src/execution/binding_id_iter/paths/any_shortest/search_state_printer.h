#include "search_state.h"

#include <vector>

#include "execution/graph_object/graph_object_manager.h"
#include "query_optimizer/rdf_model/rdf_model.h"
#include "query_optimizer/quad_model/quad_model.h"


class SearchStatePrinter {
public:
    static void get_path_quad_model(std::ostream& os, const Paths::AnyShortest::SearchState* state) {
        std::vector<ObjectId> nodes;
        std::vector<ObjectId> types;
        std::vector<bool>     directions;

        for (auto* current_state = state; current_state != nullptr; current_state = current_state->previous) {
            nodes.push_back(current_state->node_id);
            types.push_back(current_state->type_id);
            directions.push_back(current_state->inverse_direction);
        }

        os << "(" << quad_model.get_graph_object(nodes[nodes.size() - 1]) << ")";

        for (int_fast32_t i = nodes.size() - 2; i >= 0; i--) { // don't use unsigned i, will overflow
            if (directions[i]) {
                os << "<-[:" << quad_model.get_graph_object(types[i]) << "]-";
            } else {
                os << "-[:" << quad_model.get_graph_object(types[i]) << "]->";
            }
            os << "(" << quad_model.get_graph_object(nodes[i]) << ")";
        }
    };

    static void get_path_rdf_model(std::ostream& os, const Paths::AnyShortest::SearchState* state) {
        std::vector<ObjectId> nodes;
        std::vector<ObjectId> types;
        std::vector<bool>     directions;

        for (auto* current_state = state; current_state != nullptr; current_state = current_state->previous) {
            nodes.push_back(current_state->node_id);
            types.push_back(current_state->type_id);
            directions.push_back(current_state->inverse_direction);
        }

        os << '(';
        GraphObjectManager::print_rdf_compressed(os, rdf_model.get_graph_object(nodes[nodes.size() - 1])) ;
        os << ')';

        for (int_fast32_t i = nodes.size() - 2; i >= 0; i--) { // don't use unsigned i, will overflow
            if (directions[i]) {
                os << "<-[";
                GraphObjectManager::print_rdf_compressed(os, rdf_model.get_graph_object(types[i]));
                os << "]-";
            } else {
                os << "-[";
                GraphObjectManager::print_rdf_compressed(os, rdf_model.get_graph_object(types[i]));
                os << "]->";
            }
            os << '(';
            GraphObjectManager::print_rdf_compressed(os, rdf_model.get_graph_object(nodes[i]));
            os << ')';
        }
    }
};