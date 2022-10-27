#include "sparql_path_plan.h"

#include <variant>

#include "base/exceptions.h"
#include "base/query/sparql/sparql_element.h"
#include "base/query/sparql/path.h"
#include "query_optimizer/rdf_model/rdf_model.h"

using namespace std;

SparqlPathPlan::SparqlPathPlan(Id subject, SPARQL::IPath& path, Id object) :
    subject (subject),
    path    (path),
    object  (object) { }

double SparqlPathPlan::estimate_cost() const {
    // TODO: find a better estimation
    if (!subject_assigned && !object_assigned) {
        return std::numeric_limits<double>::max();
    }
    return /*100.0 +*/ estimate_output_size();
}


void SparqlPathPlan::print(std::ostream& os, int indent, const std::vector<std::string>& var_names) const {
    for (int i = 0; i < indent; ++i) {
        os << ' ';
    }
    os << "SparqlPathPlan(";
    if (std::holds_alternative<ObjectId>(subject)) {
        os << "subject: " << rdf_model.get_graph_object(std::get<ObjectId>(subject));
    } else {
        os << "subject: " << var_names[std::get<VarId>(subject).id];
    }
    if (std::holds_alternative<ObjectId>(object)) {
        os << ", object: " << rdf_model.get_graph_object(std::get<ObjectId>(object));
    } else {
        os << ", object: " << var_names[std::get<VarId>(object).id];
    }
    // os << ", path: " << var_names[path_var.id] << ": " << path.to_string();
    os << ")";

    os << ",\n";
    for (int i = 0; i < indent; ++i) {
        os << ' ';
    }
    os << "  ↳ Estimated factor: " << estimate_output_size();
}


double SparqlPathPlan::estimate_output_size() const {
    // TODO: find a better estimation
    const auto total_triples = static_cast<double>(rdf_model.catalog().triples_count);
    return total_triples * total_triples;
}


std::set<VarId> SparqlPathPlan::get_vars() const {
    std::set<VarId> result;
    if (std::holds_alternative<VarId>(subject) && !subject_assigned) {
        result.insert(std::get<VarId>(subject));
    }
    if (std::holds_alternative<VarId>(object) && !object_assigned) {
        result.insert(std::get<VarId>(object));
    }

    return result;
}


void SparqlPathPlan::set_input_vars(const std::set<VarId>& input_vars) {
    set_input_var(input_vars, subject, &subject_assigned);
    set_input_var(input_vars, object,  &object_assigned);
}


unique_ptr<BindingIdIter> SparqlPathPlan::get_binding_id_iter(ThreadInfo* thread_info) const {
    std::function<ObjectId(const std::string&)> str_to_object_id_f = [](const std::string& str) {
        return rdf_model.get_object_id(SparqlElement(Iri(str)));
    };

    auto automaton = path.get_rpq_automaton(str_to_object_id_f);

    automaton.print();

    // if (path_semantic == PathSemantic::ANY) {
    //     if (from_assigned) {
    //         auto automaton = path.get_rpq_automaton(str_to_object_id_f);
    //         if (to_assigned) {
    //             // bool case
    //             return make_unique<Paths::AnyShortest::BFSCheck>(thread_info,
    //                                                              path_var,
    //                                                              from,
    //                                                              to,
    //                                                              automaton);
    //         } else {
    //             // enum starting on from
    //             return make_unique<Paths::AnyShortest::BFSIterEnum>(thread_info,
    //                                                                 path_var,
    //                                                                 from,
    //                                                                 std::get<VarId>(to),
    //                                                                 automaton);
    //         }
    //     } else {
    //         if (to_assigned) {
    //             // enum starting on to
    //             auto inverted_path = path.invert();
    //             auto automaton     = inverted_path->get_rpq_automaton(str_to_object_id_f);
    //             return make_unique<Paths::AnyShortest::BFSIterEnum>(thread_info,
    //                                                                 path_var,
    //                                                                 to,
    //                                                                 std::get<VarId>(from),
    //                                                                 automaton);
    //         } else {
    //             if (path.nullable()) {
    //                 throw QuerySemanticException("Nullable property paths must have at least 1 node fixed");
    //             }
    //             auto automaton = path.get_rpq_automaton(str_to_object_id_f);
    //             return make_unique<Paths::AnyShortest::UnfixedComposite>(thread_info,
    //                                                                      path_var,
    //                                                                      std::get<VarId>(from),
    //                                                                      std::get<VarId>(to),
    //                                                                      automaton);
    //         }
    //     }
    // } else {
    //     // ALL SHORTEST
    //     if (from_assigned) {
    //         auto automaton = path.get_rpq_automaton(str_to_object_id_f);
    //         if (to_assigned) {
    //             // bool case
    //             return make_unique<Paths::AllShortest::BFSCheck>(thread_info,
    //                                                              path_var,
    //                                                              from,
    //                                                              to,
    //                                                              automaton);
    //         } else {
    //             // enum starting on from
    //             return make_unique<Paths::AllShortest::BFSEnum>(thread_info,
    //                                                             path_var,
    //                                                             from,
    //                                                             std::get<VarId>(to),
    //                                                             automaton);
    //         }
    //     } else {
    //         if (to_assigned) {
    //             // enum starting on to
    //             auto inverted_path = path.invert();
    //             auto automaton     = inverted_path->get_rpq_automaton(str_to_object_id_f);
    //             return make_unique<Paths::AllShortest::BFSEnum>(thread_info,
    //                                                             path_var,
    //                                                             to,
    //                                                             std::get<VarId>(from),
    //                                                             automaton);
    //         } else {
    //             // TODO: allow no-nullable unfixed paths
    //             throw QuerySemanticException("property paths must have at least 1 node fixed.");
    //         }
    //     }
    // }
    return nullptr;
}
