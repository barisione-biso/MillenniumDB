#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "file/page.h"
#include "file/buffer_manager.h"


BufferManager::BufferManager()
{

}

int BufferManager::count_pages(const std::string& filename)
{
    return boost::filesystem::file_size(filename)/PAGE_SIZE;
}

Page& BufferManager::append_page(const std::string& filename)
{
    return get_page(file_page_size[filename] ,filename);
}

Page& BufferManager::get_page(int page_number, const std::string& filename) {
    std::pair<std::string, int> page_key = std::make_pair(filename, page_number);
    if (file_page_size.find(filename) == file_page_size.end()) { // file is not open yet
        if (!boost::filesystem::exists(filename)) { // file do not exist
            // TODO: if checking throw exceptio
            if (page_number != 0){
                std::cout << "INCONSISTENCIA: se llama a BufferManager::get_page de un archivo inexistente con page_number != 0\n";
            }
            Page& new_page = *new Page(filename);
            file_page_size.insert(std::pair<std::string, int>(filename, 1));
            pages.insert(std::make_pair(page_key, &new_page));
            return new_page;
        }
        else { // file exists
            int max_page = count_pages(filename);
            if (max_page <= page_number) {
                // need to resize
                // TODO: if checking throw exception if inserting more than one page
                boost::filesystem::resize_file(filename, PAGE_SIZE*(page_number+1));
                file_page_size.insert(std::pair<std::string, int>(filename, page_number+1));
            }
            else {
                file_page_size.insert(std::pair<std::string, int>(filename, max_page));
            }
            Page& new_page = *new Page(page_number, filename);
            pages.insert(std::make_pair(page_key, &new_page));
            return new_page;
        }
    }
    else { // file is already open
        // check if page is in buffer
        std::map<std::pair<std::string, int>, Page*>::iterator it = pages.find(std::pair<std::string, int>(filename, page_number));
        if (it == pages.end()) { // page not in buffer
            int max_page = count_pages(filename);
            if (max_page <= page_number) {
                // need to resize
                // TODO: if checking throw exception if inserting more than one page
                boost::filesystem::resize_file(filename, PAGE_SIZE*(page_number+1));
                file_page_size[filename] = page_number+1;
            }
            Page& new_page = *new Page(page_number, filename);
            pages.insert(std::make_pair(page_key, &new_page));
            return new_page;
        }
        else { // page in buffer
            return *(it->second);
        }
    }
}