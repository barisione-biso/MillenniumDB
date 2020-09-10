#ifndef RELATIONAL_MODEL__GRAPH_MODEL_H_
#define RELATIONAL_MODEL__GRAPH_MODEL_H_

#include <memory>

#include "base/binding/binding_iter.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"

class GraphModel {
public:
    virtual ~GraphModel() = default;

    virtual std::unique_ptr<BindingIter> exec(OpSelect&) = 0;
    virtual std::unique_ptr<BindingIter> exec(manual_plan_ast::Root&) = 0;
    virtual std::shared_ptr<GraphObject> get_graph_object(ObjectId) = 0;
};

#endif // RELATIONAL_MODEL__GRAPH_MODEL_H_