#ifndef FILE__PAGE_MANAGER_
#define FILE__PAGE_MANAGER_

#include <map>
#include <set>
#include <string>

class Page;

class BufferManager {
public:
    BufferManager();
    ~BufferManager() = default;
    Page& get_page(int page_number, const std::string& filename);
    Page& append_page(const std::string& filename);

private:
    int count_pages(const std::string& filename);
    std::map<std::string, int> file_page_size;
    std::map<std::pair<std::string, int>, Page*> pages;

};

#endif //FILE__PAGE_MANAGER_
