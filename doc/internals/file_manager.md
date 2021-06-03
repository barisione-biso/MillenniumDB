# File Manager
The file manager is a global object that manages all the interactions with files.

- The global object is initialized when a model (e.g. `QuadModel`) is constructed.

- When working with files, you need to ask for a FileId or TmpFile.

    - If you want to use your private buffer (depends on each thread) then you need to ask for a TmpFile,
    otherwise you can ask for a FileId

    - Most of the time you use the FileId ID to ask for pages in the `buffer_manager`.

    - In some cases when you don't want to use the buffer manager (maybe because your data its not divided in pages) you can ask directly for a `fstream`.
        - e.g. `auto& file = file_manager.get_file(file_id);`

- The `file_manager` mantains a list (`opened_files`) with all files that are opened, and another list (`file_paths`) with the string of the file paths. The FileId its just the index on those lists, so both lists must have the same size and objects at the same index are related to each other.

- When a file is removed from the list, its FileId is added to the queue `available_file_ids` to reuse that FileId later. If we didn't reuse FileIds, receiving queries that use temporary files could increase the list size indefinitely.

- The instance `file_manager` cannot be destroyed before the `buffer_manager` flushes the dirty pages on exit because the `buffer_manager` needs to access the file paths in that process.
