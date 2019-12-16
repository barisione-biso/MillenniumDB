#ifndef GRAPH_SCAN_H_
#define GRAPH_SCAN_H_

#include "file/index/bplus_tree/bplus_tree.h"
// #include "relational_model/physical_plan/binding_id_range.h"
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
    GraphScan(int graph_id, BPlusTree& bpt, std::vector<ObjectId> terms, std::vector<VarId> vars);
    ~GraphScan() = default;
    void init(std::shared_ptr<BindingId> input);
    void reset(std::shared_ptr<BindingId> input);
    std::unique_ptr<BindingId> next();

private:
    int graph_id;
    int record_size;
    BPlusTree& bpt;
    std::vector<ObjectId> terms;
    std::vector<VarId> vars;
    std::unique_ptr<BPlusTree::Iter> it;
    std::shared_ptr<BindingId> input;
};

#endif //GRAPH_SCAN_H_
