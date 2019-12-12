/*
 * FileManager mantains a list (`opened_files`) with all files that were opened (even if the file is closed),
 * and another list (`filenames`) with the string of the file paths. Both list must have the same size
 * and their objects at the same index are related to each other.
 *
 * All the other clases that need working with files should use the FileManager to obtain a reference to a
 * fstream.
 *
 * public methods from FileManager are static and we should have only 1 static instance of this object,
 * which is created before the first line of code executes and destroyed at the end of the execution.
 * the singleton `instance` cannot be destroyed before the BufferManager flushes its dirty pages on exit
 * because BufferManager needs to access the file paths from FileManager.
 *
 * We use a FileId outside of FileManager as a unique identifier to the file, because storing the filename
 * string as a value use more memory than needed; and storing a reference to a string can be problematic when
 * destructors are called at the end of the execution (specially important when flushing the BufferManager).
 */

#ifndef FILE__FILE_MANAGER_H_
#define FILE__FILE_MANAGER_H_

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <climits>

struct FileId {
    static const uint_fast32_t UNASSIGNED = -1;

    uint_fast32_t id;
    FileId(uint_fast32_t _id) {
        id = _id;
    }
    bool operator<(const FileId other) const {
        return this->id < other.id;
    }
};

class FileManager {
friend class BufferManager;

public:
    static FileId get_file_id(const std::string& filename);

    static std::string get_filename(FileId file_id);
    static std::fstream& get_file(FileId file_path);
    static uint_fast32_t count_pages(FileId file_id);
    static void ensure_open(FileId file_id); // will reopen file if it was closed
    static void close(FileId file_id);
    static void remove(FileId file_id);
    static void rename(FileId old_name_id, FileId new_name_id);

    static void read_page(FileId file_id, uint_fast32_t page_number, char* bytes);
    static void flush(FileId file_id, uint_fast32_t page_number, char* bytes);

private:
    static FileManager instance; // static singleton
    bool flushed_at_exit = false;
    std::vector<std::fstream*> opened_files;
    std::vector<std::string> filenames;

    FileManager();
    ~FileManager();

    FileId _get_file_id(const std::string& filename);
};

#endif //FILE__FILE_MANAGER_H_
