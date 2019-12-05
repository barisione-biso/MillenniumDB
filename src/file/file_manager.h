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
    static void ensure_open(FileId file_id); // reopen if it was closed
    static void close(FileId file_id);
    static void remove(FileId file_id);
    static void rename(FileId old_name_id, FileId new_name_id);

    static void read_page(FileId file_id, uint_fast32_t page_number, char* bytes);
    static void flush(FileId file_id, uint_fast32_t page_number, char* bytes);
private:
    static FileManager instance;
    bool flushed_at_exit = false;
    std::vector<std::fstream*> opened_files;
    std::vector<std::string> filenames;

    FileManager();
    ~FileManager();

    FileId _get_file_id(const std::string& filename);
};

#endif //FILE__FILE_MANAGER_H_
