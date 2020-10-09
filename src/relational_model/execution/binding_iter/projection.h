#ifndef RELATIONAL_MODEL__PROJECTION_H_
#define RELATIONAL_MODEL__PROJECTION_H_

#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "relational_model/execution/binding/binding_id_iter.h"

class Projection : public BindingIter {

private:
    std::vector<std::string> projection_vars;
    std::unique_ptr<BindingIter> iter;
    uint_fast32_t limit;
    uint_fast32_t count;

public:
    Projection(std::unique_ptr<BindingIter> iter, std::vector<std::string> projection_vars, uint_fast32_t limit);
    ~Projection();

    void begin();
    std::unique_ptr<Binding> next();

    // prints execution statistics
    void analyze(int indent = 0) const override;
};

#endif // RELATIONAL_MODEL__PROJECTION_H_
