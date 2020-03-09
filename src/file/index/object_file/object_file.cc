#include "object_file.h"

#include "file/index/bplus_tree/bplus_tree.h"

#include <iostream>
#include <file/file_manager.h>

using namespace std;

ObjectFile::ObjectFile(const string& filename)
    : file (file_manager.get_file(file_manager.get_file_id(filename)))
{
    file.seekg (0, file.end);
    // If the file is empty, write a trash byte to prevent the ID = 0
    if (file.tellg() == 0) {
        char c = 0;
        file.write(&c, 1);
    }
}


unique_ptr<vector<unsigned char>> ObjectFile::read(uint64_t id) {
    // set position=`id` and read the length of the object
    file.seekg(id);
    char length_b[4]; // array of 4 bytes to store the length as bytes
    file.read(length_b, 4);
    uint32_t length = *(uint32_t *)length_b;

    // TODO: check sanity
    // if (file.bad())
    //     std::cout << "I/O bad\n";
    // else if (file.eof())
    //     std::cout << "I/O eof\n";
    // else if (file.fail())
    //     std::cout << "I/O fail\n";

    // read the following `length` bytes
    auto value = make_unique<vector<unsigned char>>(length);
    file.read((char*) value->data(), length);
    return value;
}


uint64_t ObjectFile::write(vector<unsigned char>& bytes) {
    // set position at end of file
    file.seekg(0, file.end);

    // remember the actual position where we are writing the object
    uint64_t write_pos = file.tellg();

    // write to the file the length in bytes
    uint32_t length = bytes.size();
    file.write((const char *)&length, 4);

    // write to the file the bytes of the object
    file.write((const char*)bytes.data(), length);

    return write_pos;
}
