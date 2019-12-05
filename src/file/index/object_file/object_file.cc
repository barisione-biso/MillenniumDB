#include "file/index/object_file/object_file.h"

#include <iostream>
#include <file/file_manager.h>

using namespace std;

ObjectFile::ObjectFile(const string& filename)
    : file (FileManager::get_file(FileManager::get_file_id(filename)))
{
    file.seekg (0, file.end);
    if (file.tellg() == 0) { // Write trash to prevent the id = 0
        char c = 0;
        file.write(&c, 1);
    }
}

ObjectFile::~ObjectFile()
{
    file.close();
}

unique_ptr<vector<char>> ObjectFile::read(uint64_t id)
{
    file.seekg(id);

    char length_b[4];
    file.read(length_b, 4);

    int length = *(int *)length_b;
    // TODO: check if length is reasonable to check for possible errors?
    auto value = make_unique<vector<char>>(length);
    file.read(&(*value)[0], length);

    return value;
}

uint64_t ObjectFile::write(vector<char>& bytes){
    int length = bytes.size();

    file.seekg(0, file.end);
    int write_pos = file.tellg();

    file.write((const char *)&length, 4);
    file.write(bytes.data(), length);
    return write_pos;
}
