#ifndef RELATIONAL_MODEL__PROJECTION_H_
#define RELATIONAL_MODEL__PROJECTION_H_

#include "base/binding/binding_iter.h"
#include "relational_model/physical_plan/binding_id_iter.h"

#include <memory>
#include <map>

class Projection : public BindingIter {

private:
    ObjectFile& obj_file;
    std::unique_ptr<BindingIdIter> iter;
    std::map<std::string, std::string> constants;
    std::vector<std::string> names;
    std::vector<int_fast32_t> var_positions;
    int_fast32_t global_vars;
    std::unique_ptr<BindingId> root_input;

public:
    Projection(
        ObjectFile& obj_file,
        std::unique_ptr<BindingIdIter> iter,
        std::map<std::string, std::string> constants,
        std::vector<std::string> names,
        std::vector<int_fast32_t> var_positions,
        int_fast32_t global_vars);
    ~Projection() = default;
    void init();
    Binding* next();
};

#endif //RELATIONAL_MODEL__PROJECTION_H_
