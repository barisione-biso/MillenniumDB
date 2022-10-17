#include "object_file.h"

#include <cassert>
#include <cstring>

#include "base/exceptions.h"
#include "storage/file_manager.h"
#include "query_optimizer/rdf_model/rdf_model.h"

using namespace std;

ObjectFile::ObjectFile(const string& filename)
{
    auto file_path = file_manager.get_file_path(filename);
    file.open(file_path, ios::out|ios::app);
    if (file.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    file.close();
    file.open(file_path, ios::in|ios::out|ios::binary);

    file.seekg (0, file.end);
    auto end_pos = file.tellg();

    if (end_pos == 0) {
        // The object file doesn't exist
        capacity = INITIAL_SIZE;
        objects = new char[INITIAL_SIZE];
        objects[0] = '\0'; // skip the first byte to prevent the ID = 0
        current_end = 1;
    } else {
        // The object already exists
        capacity = end_pos;
        objects = new char[end_pos];
        file.seekg(0, file.beg);
        file.read(objects, end_pos);
        current_end = end_pos;
    }
}


ObjectFile::~ObjectFile() {
    file.seekg(0, file.beg);
    file.write(objects, current_end);
    file.close();
    delete[] objects;
}


std::string ObjectFile::get_string(uint64_t id) const {
    assert(id < current_end);
    return std::string(&objects[id]);
}

std::string& ObjectFile::get_prefix(uint8_t prefix_id) const {
    assert(prefix_id < rdf_model.catalog().prefixes.size());
    return rdf_model.catalog().prefixes[prefix_id];
}

std::string& ObjectFile::get_datatype(uint16_t datatype_id) const {
    return rdf_model.catalog().datatypes[datatype_id];
}

std::string& ObjectFile::get_language(uint16_t language_id) const {
    return rdf_model.catalog().languages[language_id];
}

void ObjectFile::print_string(std::ostream& os, uint64_t id) const {
    assert(id < current_end);
    os << &objects[id];
}

uint64_t ObjectFile::write(const std::string& str) {
    assert(str.size() >= 8); // 7 or less bytes can be inlined

    uint64_t write_pos = current_end;
    // check the is enough space
    while (current_end + str.size() + 1 >= capacity) {
        // duplicate buffer
        char* new_objects = new char[capacity*2];
        std::memcpy(
            new_objects,
            objects,
            capacity);

        capacity *= 2;

        delete[] objects;
        objects = new_objects;
    }
    // write
    std::memcpy(
        &objects[current_end],
        str.data(),
        str.size());

    current_end += str.size();
    objects[current_end] = '\0';
    ++current_end;

    return write_pos;
}
