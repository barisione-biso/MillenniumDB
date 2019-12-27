#ifndef GRAPH_SCAN_H_
#define GRAPH_SCAN_H_

#include "file/index/bplus_tree/bplus_tree.h"
#include "relational_model/physical_plan/binding_id_iter/binding_id_iter.h"

#include <functional>
#include <list>
#include <memory>
#include <tuple>

class Record;
class VarId;
class GraphScan : public BindingIdIter
{
public:
    GraphScan(BPlusTree& bpt, std::vector<std::pair<ObjectId, int>> terms, std::vector<std::pair<VarId, int>> vars);
    ~GraphScan() = default;
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

#endif //GRAPH_SCAN_H_
