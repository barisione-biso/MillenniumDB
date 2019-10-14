#include "file/index/object_file/object_file.h"

#include <iostream>

using namespace std;

ObjectFile::ObjectFile(const string& filename)
{
    file = fstream(filename, ios::in|ios::out|ios::binary);
    file.seekg (0, file.end);
    int last_pos = file.tellg();
    if (last_pos == -1) {
        file.close();
        std::ofstream tmp(filename);
        tmp.close();
        file = fstream(filename, ios::in|ios::out|ios::binary);
    }
}

ObjectFile::~ObjectFile()
{
    file.close();
}

unique_ptr<vector<char>> ObjectFile::read(u_int64_t id)
{
    file.seekg(id);

    char length_b[4];
    file.read(length_b, 4);

    int length = *(int *)length_b;
    auto value = make_unique<vector<char>>(length);
    file.read(&(*value)[0], length);

    return value;
}

u_int64_t ObjectFile::write(vector<char>& bytes){
    int length = bytes.size();

    file.seekg(0, file.end);
    int write_pos = file.tellg();

    file.write((const char *)&length, 4);
    file.write(bytes.data(), length);
    return write_pos;
}
