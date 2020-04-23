#include <fstream>
#include <iostream>

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "relational_model/relational_model.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/physical_plan_generator.h"

#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

int main() {
    // TODO: use program options for database folder
    file_manager.init();
    buffer_manager.init();
    relational_model.init();

    std::cout << "Checking hash2id\n";
    if (!relational_model.get_hash2id_bpt().check())
        std::cout << "wrong BPlusTree: hash2id\n";

    // NODE LABELS
    std::cout << "Checking label2node\n";
    if (!relational_model.get_label2node().check())
        std::cout << "wrong BPlusTree: label2node\n";
    std::cout << "Checking node2label\n";
    if (!relational_model.get_node2label().check())
        std::cout << "wrong BPlusTree: node2label\n";

    // EDGE LABELS
    std::cout << "Checking label2edge\n";
    if (!relational_model.get_label2edge().check())
        std::cout << "wrong BPlusTree: label2edge\n";
    std::cout << "Checking edge2label\n";
    if (!relational_model.get_edge2label().check())
        std::cout << "wrong BPlusTree: edge2label\n";

    // NODE PROPERTIES
    std::cout << "Checking key_value_node\n";
    if (!relational_model.get_key_value_node().check())
        std::cout << "wrong BPlusTree: key_value_node\n";
    std::cout << "Checking key_node_value\n";
    if (!relational_model.get_key_node_value().check())
        std::cout << "wrong BPlusTree: key_node_value\n";
    std::cout << "Checking node_key_value\n";
    if (!relational_model.get_node_key_value().check())
        std::cout << "wrong BPlusTree: node_key_value\n";

    // EDGE PROPERTIES
    std::cout << "Checking key_value_edge\n";
    if (!relational_model.get_key_value_edge().check())
        std::cout << "wrong BPlusTree: key_value_edge\n";
    std::cout << "Checking key_edge_value\n";
    if (!relational_model.get_key_edge_value().check())
        std::cout << "wrong BPlusTree: key_edge_value\n";
    std::cout << "Checking edge_key_value\n";
    if (!relational_model.get_edge_key_value().check())
        std::cout << "wrong BPlusTree: edge_key_value\n";

    // CONNECTIONS
    std::cout << "Checking from_to_edge\n";
    if (!relational_model.get_from_to_edge().check())
        std::cout << "wrong BPlusTree: from_to_edge\n";
    std::cout << "Checking to_edge_from\n";
    if (!relational_model.get_to_edge_from().check())
        std::cout << "wrong BPlusTree: to_edge_from\n";
    std::cout << "Checking edge_from_to\n";
    if (!relational_model.get_edge_from_to().check())
        std::cout << "wrong BPlusTree: edge_from_to\n";
}
