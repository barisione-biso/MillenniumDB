#ifndef RELATIONAL_MODEL__OPTIONAL_H_
#define RELATIONAL_MODEL__OPTIONAL_H_

#include "file/index/bplus_tree/bplus_tree.h"
#include "relational_model/physical_plan/binding_id_iter/binding_id_iter.h"

#include <functional>
#include <list>
#include <memory>
#include <tuple>

class Record;
class VarId;
class Optional : public BindingIdIter
{
public:
    Optional(BPlusTree& bpt, std::vector<std::pair<ObjectId, int>> terms, std::vector<std::pair<VarId, int>> vars);
    ~Optional() = default;
    void init(std::shared_ptr<BindingId> input);
    void reset(std::shared_ptr<BindingId> input);
    std::unique_ptr<BindingId> next();

private:
    int record_size;
    BPlusTree& bpt;
    std::vector<std::pair<ObjectId, int>> terms;
    std::vector<std::pair<VarId, int>> vars;
    std::unique_ptr<BPlusTree::Iter> it;
    std::shared_ptr<BindingId> input;
};

#endif //RELATIONAL_MODEL__OPTIONAL_H_
