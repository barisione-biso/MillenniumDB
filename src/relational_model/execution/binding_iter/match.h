#ifndef RELATIONAL_MODEL__MATCH_H_
#define RELATIONAL_MODEL__MATCH_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"
#include "relational_model//models/graph_model.h"

class Match : public BindingIter {
public:
    Match(GraphModel& model, std::unique_ptr<BindingIdIter> root, std::map<std::string, VarId> var_pos);
    ~Match() = default;

    void begin();
    std::unique_ptr<Binding> next();

    // prints execution statistics
    void analyze(int indent = 0) const override;

private:
    GraphModel& model;
    std::unique_ptr<BindingIdIter> root;
    const std::map<std::string, VarId> var_pos;
    std::unique_ptr<BindingId> input;
    BindingId* binding_id_root;
};

#endif // RELATIONAL_MODEL__MATCH_H_
