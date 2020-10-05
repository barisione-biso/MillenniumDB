#include "quad_catalog.h"

#include <iostream>

using namespace std;

QuadCatalog::QuadCatalog(const std::string& filename) :
    Catalog(filename)
{
    if (is_empty()) {
        cout << "Initializing empty catalog.\n";
        identifiable_nodes_count = 0;
        anonymous_nodes_count    = 0;
        connections_count        = 0;

        label_count              = 0;
        properties_count         = 0;

        distinct_labels          = 0;
        distinct_keys            = 0;
        distinct_types           = 0;

        equal_from_to_count      = 0;
        equal_from_type_count    = 0;
        equal_to_type_count      = 0;
        equal_from_to_type_count = 0;
    }
    else {
        start_io();
        identifiable_nodes_count = read_uint64();
        anonymous_nodes_count    = read_uint64();
        connections_count        = read_uint64();

        label_count              = read_uint64();
        properties_count         = read_uint64();

        distinct_labels          = read_uint64();
        distinct_types           = read_uint64();
        distinct_keys            = read_uint64();

        equal_from_to_count      = read_uint64();
        equal_from_type_count    = read_uint64();
        equal_to_type_count      = read_uint64();
        equal_from_to_type_count = read_uint64();

        for (uint_fast32_t i = 0; i < distinct_labels; i++) {
            auto label_id          = read_uint64();
            auto label_total_count = read_uint64();
            label2total_count.insert({ label_id, label_total_count });
        }

        for (uint_fast32_t i = 0; i < distinct_types; i++) {
            auto type_id          = read_uint64();
            auto type_total_count = read_uint64();
            type2total_count.insert({ type_id, type_total_count });
        }

        for (uint_fast32_t i = 0; i < distinct_keys; i++) {
            auto key_id           = read_uint64();
            auto key_total_count  = read_uint64();
            key2total_count.insert({ key_id, key_total_count });
        }

        for (uint_fast32_t i = 0; i < distinct_keys; i++) {
            auto key_id             = read_uint64();
            auto key_distinct_count = read_uint64();
            key2distinct.insert({ key_id, key_distinct_count });
        }
    }

}


void QuadCatalog::save_changes() {
    print();
    start_io();

    write_uint64(identifiable_nodes_count);
    write_uint64(anonymous_nodes_count);
    write_uint64(connections_count);

    write_uint64(label_count);
    write_uint64(properties_count);

    write_uint64(distinct_labels);
    write_uint64(distinct_types);
    write_uint64(distinct_keys);

    write_uint64(equal_from_to_count);
    write_uint64(equal_from_type_count);
    write_uint64(equal_to_type_count);
    write_uint64(equal_from_to_type_count);

    for (auto&&[k, v] : label2total_count) {
        write_uint64(k);
        write_uint64(v);
    }

    for (auto&&[k, v] : type2total_count) {
        write_uint64(k);
        write_uint64(v);
    }

    for (auto&&[k, v] : key2total_count) {
        write_uint64(k);
        write_uint64(v);
    }

    for (auto&&[k, v] : key2distinct) {
        write_uint64(k);
        write_uint64(v);
    }
}


void QuadCatalog::print() {
    cout << "-------------------------------------\n";
    cout << "Catalog:\n";
    cout << "  identifiable nodes count: " << identifiable_nodes_count << "\n";
    cout << "  anonymous nodes count:    " << anonymous_nodes_count    << "\n";
    cout << "  connections count:        " << connections_count        << "\n";

    cout << "  label count:              " << label_count              << "\n";
    cout << "  properties count:         " << properties_count         << "\n";

    cout << "  disinct labels:           " << distinct_labels          << "\n";
    cout << "  disinct types:            " << distinct_types           << "\n";
    cout << "  disinct keys:             " << distinct_keys            << "\n";

    cout << "  equal_from_to_count:      " << equal_from_to_count      << "\n";
    cout << "  equal_from_type_count:    " << equal_from_type_count    << "\n";
    cout << "  equal_to_type_count:      " << equal_to_type_count      << "\n";
    cout << "  equal_from_to_type_count: " << equal_from_to_type_count << "\n";
    cout << "-------------------------------------\n";
}
