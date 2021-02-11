/*
 * FileManager mantains a list (`opened_files`) with all files that were opened (even if the file is closed),
 * and another list (`filenames`) with the string of the file paths. Both list must have the same size
 * and objects at the same index are related to each other.
 *
 * All the other clases that need to work with files should use the FileManager to obtain a reference to a
 * fstream.
 *
 * `file_manager` is a global object and is available when this file is included. Before using it, somebody
 * needs to call the method FileManager::init(), usually is the responsability of the model (e.g. RelationalModel)
 * to call it.
 *
 * The instance `file_manager` cannot be destroyed before the BufferManager flushes its dirty pages on exit
 * because BufferManager needs to access the file paths from FileManager. Nifty counter trick should handle this
 * automagically.
 */

#ifndef STORAGE__FILE_MANAGER_H_
#define STORAGE__FILE_MANAGER_H_

#include <fstream>
#include <queue>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "storage/file_id.h"
#include "storage/page_id.h"

class FileManager {
friend class Page; // to allow pages call file_manager.flush
friend class BufferManager; // to allow pages call file_manager.read_page
public:
    ~FileManager() = default;

    // necesary to be called before first usage
    static void init(const std::string& db_folder);

    // Get an id for the corresponding file, creating it if it's necessary
    FileId get_file_id(const std::string& filename);

    // get the file stream assignated to `file_id` as a reference. Only use this when not accessing via BufferManager
    std::fstream& get_file(const FileId file_path) const;

    // count how many pages a file have
    uint_fast32_t count_pages(const FileId file_id) const;

    // delete the file represented by `file_id`, pages in buffer using that file_id are cleared
    void remove(const FileId file_id);

private:
    // folder where all the used files will be
    const std::string db_folder;

    // contains all file streams that have been opened, including closed ones
    std::vector< std::unique_ptr<std::fstream> > opened_files;

    std::queue<FileId> available_file_ids;

    std::map<std::string, FileId> filename2file_id;

    // to avoid synchronization problems when establishing a new file_id in `get_file_id(filename)`
    std::mutex files_mutex;

    // contains all filenames that have been used. The position in this vector is equivalent to the FileId
    // representing that file
    std::vector<std::string> file_paths;

    // private constructor, other classes must use the global object `file_manager`
    FileManager(const std::string& db_folder);

    // write the data pointed by `bytes` page represented by `page_id` to disk.
    // `bytes` must point to the start memory position of `Page::PAGE_SIZE` allocated bytes
    void flush(PageId page_id, char* bytes) const;

    // read a page from disk into memory pointed by `bytes`.
    // `bytes` must point to the start memory position of `Page::PAGE_SIZE` allocated bytes
    void read_page(PageId page_id, char* bytes) const;
};

extern FileManager& file_manager; // global object

#endif // STORAGE__FILE_MANAGER_H_
