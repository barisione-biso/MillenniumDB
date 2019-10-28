#ifndef FILE__BUFFER_MANAGER_H_
#define FILE__BUFFER_MANAGER_H_

#include <map>
#include <set>
#include <string>

class Page;
class Buffer;

class BufferManager {
public:
    BufferManager();
    ~BufferManager();
    Page& get_page(int page_number, const std::string& filename);
    Page& append_page(const std::string& filename);

private:
    int count_pages(const std::string& filename);
    // std::map<std::string, int> file_page_size;
    std::map<std::pair<std::string, int>, int> pages;

    Buffer** buffer_pool; // array of pointers
    Page**   pages_pool; // array of pointers
    int clock_pos;
    int get_buffer_available();
};

#endif //FILE__BUFFER_MANAGER_H_
