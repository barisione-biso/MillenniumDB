#ifndef RELATIONAL_MODEL__OPTIONAL_NODE_H_
#define RELATIONAL_MODEL__OPTIONAL_NODE_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"

class OptionalNode : public BindingIdIter {
public:
    OptionalNode(std::size_t binding_size,
                        std::unique_ptr<BindingIdIter> basic_graph_pattern,
                        std::vector<std::unique_ptr<BindingIdIter>> children);
    ~OptionalNode() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
    bool has_result;

private:
    std::unique_ptr<BindingIdIter> basic_graph_pattern;
    std::size_t binding_size;
    //std::vector<std::unique_ptr<BindingIdIter>> children;

    BindingId* current_left;
    BindingId* current_right;
};

#endif // RELATIONAL_MODEL__OPTIONAL_NODE_H_
