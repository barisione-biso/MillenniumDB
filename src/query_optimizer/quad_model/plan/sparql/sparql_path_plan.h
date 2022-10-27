#pragma once

#include "base/query/sparql/path.h"
#include "query_optimizer/quad_model/plan/plan.h"

class SparqlPathPlan : public Plan {
public:
    SparqlPathPlan(Id subject, SPARQL::IPath& path, Id object);

    SparqlPathPlan(const SparqlPathPlan& other) :
        subject            (other.subject),
        path               (other.path),
        object             (other.object),
        subject_assigned   (other.subject_assigned),
        object_assigned    (other.object_assigned) { }

    std::unique_ptr<Plan> duplicate() const override {
        return std::make_unique<SparqlPathPlan>(*this);
    }

    double estimate_cost() const override;
    double estimate_output_size() const override;

    std::set<VarId> get_vars() const override;
    void            set_input_vars(const std::set<VarId>& input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter(ThreadInfo*) const override;

    bool get_leapfrog_iter(ThreadInfo*,
                           std::vector<std::unique_ptr<LeapfrogIter>>&,
                           std::vector<VarId>&,
                           uint_fast32_t&) const override { return false; }

    void print(std::ostream& os, int indent, const std::vector<std::string>& var_names) const override;

private:
    Id subject;
    SPARQL::IPath& path;
    Id object;

    bool subject_assigned;
    bool object_assigned;
};
