#ifndef FILE__INDEX__OBJECT_FILE_
#define FILE__INDEX__OBJECT_FILE_

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class Node;

using namespace std;

class ObjectFile
{
    public:
        ObjectFile(const string& filename);
        ~ObjectFile();
        unique_ptr<vector<char>> read(u_int64_t id);
        u_int64_t write(vector<char>& bytes);
    private:
        std::fstream file;
};

#endif //FILE__INDEX__OBJECT_FILE_
