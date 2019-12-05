#ifndef TESTS__GRAPH_IMPORT_H_
#define TESTS__GRAPH_IMPORT_H_

#include <string>

class TestGraphImport {
public:
    static void bulk_import(std::string nodes_file, std::string edges_file);
    static void test_element2label();
    static void test_label2element();
};

#endif //TESTS__GRAPH_IMPORT_H_