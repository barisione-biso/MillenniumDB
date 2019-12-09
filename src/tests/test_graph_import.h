#ifndef TESTS__GRAPH_IMPORT_H_
#define TESTS__GRAPH_IMPORT_H_

#include <string>

class TestGraphImport {
public:
    static void bulk_import(std::string nodes_file, std::string edges_file);
    static void test_element2label();
    static void test_label2element();

    static void test_element2prop();
    static void test_prop2element();

    static void test_from_to_edge();
    static void test_to_from_edge();
};

#endif //TESTS__GRAPH_IMPORT_H_