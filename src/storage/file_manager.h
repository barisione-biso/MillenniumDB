/* !!!! IMPORTANT !!!! NEVER include this file in a header file
 *
 * FileManager mantains a list (`opened_files`) with all files that were opened (even if the file is closed),
 * and another list (`filenames`) with the string of the file paths. Both list must have the same size
 * and their objects at the same index are related to each other.
 *
 * All the other clases that need working with files should use the FileManager to obtain a reference to a
 * fstream.
 *
 * public methods from FileManager are static and we should have only 1 static instance of this object,
 * which is created before the first line of code executes and destroyed at the end of the execution.
 * the instance `file_manager` cannot be destroyed before the BufferManager flushes its dirty pages on exit
 * because BufferManager needs to access the file paths from FileManager.
 *
 * We use a FileId outside of FileManager as a unique identifier to the file, because storing the filename
 * string as a value use more memory than needed; and storing a reference to a string can be problematic when
 * destructors are called at the end of the execution.
 */

#ifndef STORAGE__FILE_MANAGER_H_
#define STORAGE__FILE_MANAGER_H_

#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "storage/file_id.h"

class FileManager {

public:
    FileId get_file_id(const std::string& filename);

    std::string get_filename(FileId file_id);
    std::fstream& get_file(FileId file_path);
    uint_fast32_t count_pages(FileId file_id);
    void ensure_open(FileId file_id); // will reopen file if it was closed
    void close(FileId file_id);
    void remove(FileId file_id);
    void rename(FileId old_name_id, FileId new_name_id);

    void read_page(FileId file_id, uint_fast32_t page_number, char* bytes);
    void flush(FileId file_id, uint_fast32_t page_number, char* bytes);

    FileManager();
    ~FileManager();

private:
    std::vector<std::fstream*> opened_files;
    std::vector<std::string> filenames;
};

extern FileManager& file_manager; // global object

static struct FileManagerInitializer {
    FileManagerInitializer();
    ~FileManagerInitializer();
} file_manager_initializer; // static initializer for every translation unit


#endif // STORAGE__FILE_MANAGER_H_
