#ifndef FILE__BUFFER_MANAGER_H_
#define FILE__BUFFER_MANAGER_H_

#include <map>
#include <set>
#include <string>

class Page;
class Buffer;

class BufferManager {
public:
    static void flush(); // TODO: should be automatically called, move to destructor?
    static Page& get_page(int page_number, const std::string& filename);
    static Page& append_page(const std::string& filename);

private:
    static BufferManager instance;
    std::map<std::pair<std::string, int>, int> pages;

    Page** buffer_pool; // array of pointers
    char* bytes;
    int clock_pos;

    BufferManager();
    ~BufferManager();

    void _flush();
    Page& _get_page(int page_number, const std::string& filename);
    Page& _append_page(const std::string& filename);

    int get_buffer_available();
    int count_pages(const std::string& filename);
};

#endif //FILE__BUFFER_MANAGER_H_
