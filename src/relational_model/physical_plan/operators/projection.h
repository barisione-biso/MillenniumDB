#ifndef RELATIONAL_MODEL__PROJECTION_H_
#define RELATIONAL_MODEL__PROJECTION_H_

#include "relational_model/physical_plan/binding_id_iter.h"

#include <memory>

class Record;
class VarId;
class Projection //: public BindingIter
{
public:
    Projection(std::vector<VarId> vars) {
        
    }
    ~Projection() = default;
    // void init(std::shared_ptr<BindingId> input);
    // void reset(std::shared_ptr<BindingId> input);
    // std::unique_ptr<BindingId> next();

private:
    std::vector<VarId> vars;
    std::unique_ptr<BindingIdIter> iter;
};

#endif //RELATIONAL_MODEL__PROJECTION_H_
