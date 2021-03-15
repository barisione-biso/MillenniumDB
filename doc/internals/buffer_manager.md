# Buffer Manager
The buffer manager is a global object that manages data transfers between shared memory and persistent storage.
The data transfer is meant to be in chunks of bytes with constant size called **Pages**.

Structures like B+trees and hash tables are designed to be divided in pages so it's not necessary to have the complete structure on memory, but only the pages you need. Thus allowing to handle structures bigger than memory.

* Buffer Pool can hold a constant number of pages through the execution of the program. That number can be setted at construction. // TODO: ejemplo de como pasar parametros (G, M, GB, MB, Gi, Mi GiB, MiB) en vez de páginas cuando este listo. If the buffer runs out of available pages (i.e. all the pages are pinned) an excetion will be thrown.


* Working memory used by the buffer manager is divided into two sections:
    - `bytes`: array of all the data that is persistent on disk. // TODO: change name to data?
        - Size: PAGE_SIZE * buffer_pool_size
    - `buffer_pool`: used as an array of Pages. Each Page has:
        - PageId, which is a pair (FileId, page_number)
        - pin counter
        - dirty flag
        - pointer to the disk data (`bytes`)
    - TODO: explicar que los slot de bytes y buffer_pool se relacionan 1 a 1 con su posicion correspondiente

* TODO: will we have a special buffer for a specific query execution? (e.g. for ordering tuples or hashing)


* The typicall flow is:
    - Some class need to access a page, so it ask to the buffer manager for it specifying the PageId (the page ID is a pair of FileId and page_number)
    - We use a map (std::unordered_map) to search if a page with a certain PageId is already in the buffer pool.
    - If the page is already in the buffer pool we return it and increase the pin counter.
    - If the page is not in the buffer pool we replace an unpinned page from the buffer with the asked. The page is inserted in the map, pinned and returned.

* To replace a page we use the clock replacement algorithm.
    - We have the pages in the buffer as a circular list.
    - We have a pointer `p` to remember a position in the circular list.
    - While the page pointed by `p` is not available `p` points to the next page.
    - When the page pointed by `p` is available we return that page and make `p` point to the next page.
    - If the circle completes and no page was available an exception is thrown.

* Pages are obtained via `get_page(file_id, page_number)`, (there also exists `get_last_page(file_id)` and `append_page(file_id)`, but they just call `get_page` with the correct page_number).
When you get a page, it will be pinned automatically, and its your responsability to call `unpin()` when you are done using it. Otherwise the page won't be able to be replaced in the buffer. Getting a page and unpinnig it is thread safe.

* After you get a page, the bytes you can use are in the range from `page.get_bytes()[0]` to `page.get_bytes()[Page::PAGE_SIZE - 1]`. You have te be cautious to not write outside these bounds, otherwise you may be writing in other page or in uninitialized memory because no bound checking is done. Writing and reading bytes in the page is not thread safe (for now it's not necesary because the database is created once and cannot be modified later, maybe later each page will need to have a mutex, but the buffer manager won't be resposable to lock/unlock  the page, the class that asked for a page will have the responsability to manage that if needed).

* When you modify the bytes in a page, if you want to make the changes persistent when the page is ejected from the buffer, you need to call `make_dirty()`.