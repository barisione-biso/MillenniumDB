#ifndef RELATIONAL_MODEL__MATCH_H_
#define RELATIONAL_MODEL__MATCH_H_

#include "base/binding/binding_iter.h"
#include "base/var/var_id.h"
#include "relational_model/binding/binding_id_iter.h"

#include <map>
#include <memory>
#include <vector>

class QueryOptimizerElement;
class ObjectFile;

class Match : public BindingIter {

private:
    ObjectFile& obj_file;
    std::vector<std::unique_ptr<QueryOptimizerElement>> elements;
    std::unique_ptr<BindingIdIter> root;
    std::map<std::string, VarId> var_pos;

    std::unique_ptr<BindingIdIter> get_join_plan();

public:
    Match(ObjectFile& obj_file, std::vector<std::unique_ptr<QueryOptimizerElement>> elements, std::map<std::string, VarId> var_pos);
    ~Match() = default;

    void begin();
    std::unique_ptr<Binding> next();
};

#endif //RELATIONAL_MODEL__MATCH_H_
