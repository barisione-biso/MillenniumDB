#include "sort_page.h"

#include <cassert>
#include <iostream>
#include <cstring>
#include <math.h>

#include "storage/file_id.h"
#include "storage/file_manager.h"

/*
Duda:
- Será necesario implemetar una especie de SortFile? -> La idea es que este se encargue de
manejar todas las páginas
- Implementación asume al hacer sort que todas las tuplas miden lo mismo, se debería manejar el caso en que sean distintas?
*/

int N_PAGE_SIZE = 6;
int DIRSIZE_SIZE = 2;
int FREESPACE_SIZE = 2;
int POINTER_SIZE = 2;
int SIZE_SIZE = 2;
int ENTRY_SIZE = POINTER_SIZE + SIZE_SIZE;
int ID_SIZE = sizeof(uint_fast64_t);
int PAGE_SIZE = 4096;

SortPage::SortPage(Page& page) :
    page(page)
{
    char* start_pointer = page.get_bytes();
    uint_fast16_t dirsize = 0;
    int freespace = PAGE_SIZE - N_PAGE_SIZE - DIRSIZE_SIZE - FREESPACE_SIZE;
    int PAGE_ID = 0; // Sacar id de la pagina
    memcpy(start_pointer, &PAGE_ID, N_PAGE_SIZE);  // pageno
    memcpy(start_pointer + N_PAGE_SIZE, &dirsize, DIRSIZE_SIZE); // dirsize
    memcpy(start_pointer + N_PAGE_SIZE + DIRSIZE_SIZE, &freespace, FREESPACE_SIZE); // freespace
}

SortPage::~SortPage() = default;

void SortPage::make_dirty() {
    page.make_dirty();
}

void SortPage::unpin() {

}

bool SortPage::has_space_for(uint_fast64_t space) {
    char* start_pointer = page.get_bytes();
    uint_fast64_t freespace = 0;
    memcpy(&freespace, start_pointer + N_PAGE_SIZE + DIRSIZE_SIZE, FREESPACE_SIZE);
    if (space > freespace) { return false; }
    return true;
}

uint_fast16_t SortPage:: get_dirsize(){
    char* start_pointer = page.get_bytes();
    uint_fast16_t dirsize = 0;
    memcpy(&dirsize, start_pointer + N_PAGE_SIZE, DIRSIZE_SIZE);
    return dirsize;
}
uint_fast16_t SortPage:: get_freespace(){
    char* start_pointer = page.get_bytes();
    uint_fast16_t freespace = 0;
    memcpy(&freespace, start_pointer + N_PAGE_SIZE + DIRSIZE_SIZE, FREESPACE_SIZE);
    return freespace;
}

void SortPage::write(std::vector<uint_fast64_t>& bytes) {
    uint_fast16_t size = bytes.size();
    uint_fast16_t freespace = get_freespace();
    uint_fast16_t dirsize = get_dirsize();
    char* start_pointer = page.get_bytes();
    int tuple_size = size * ID_SIZE;
    uint_fast16_t pointer = (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + (ENTRY_SIZE) * dirsize + freespace - tuple_size;
    memcpy(start_pointer + pointer, bytes.data(), tuple_size);
    memcpy(start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + ENTRY_SIZE * dirsize,&pointer, POINTER_SIZE);
    memcpy(start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE + POINTER_SIZE) + ENTRY_SIZE * dirsize,&tuple_size, SIZE_SIZE);
    freespace = freespace - tuple_size - ENTRY_SIZE;
    dirsize++;
    memcpy(start_pointer + N_PAGE_SIZE,&dirsize,DIRSIZE_SIZE);
    memcpy(start_pointer + N_PAGE_SIZE + DIRSIZE_SIZE,&freespace,FREESPACE_SIZE);
    page.make_dirty();
}

void SortPage::override_tuple(std::vector<uint_fast64_t>& tuple, int position) {
    char* start_pointer = page.get_bytes();
    uint_fast16_t pointer = 0;
    //uint_fast16_t size = 0;
    memcpy(&pointer, start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + position * (ENTRY_SIZE), POINTER_SIZE);
    //memcpy(&size, start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE + POINTER_SIZE) + position * ENTRY_SIZE, SIZE_SIZE);
    int tuple_size = tuple.size() * ID_SIZE;
    //uint_fast16_t new_pointer = (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + (ENTRY_SIZE) * position + freespace - tuple_size;
    memcpy(start_pointer + pointer, tuple.data(), tuple_size);
    //memcpy(start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + ENTRY_SIZE * position,&pointer, POINTER_SIZE);
    //memcpy(start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE + POINTER_SIZE) + ENTRY_SIZE * position,&tuple_size, SIZE_SIZE);
    page.make_dirty();
}


std::vector<uint_fast64_t>  SortPage::get(int id) {
    uint_fast16_t pointer = 0;
    uint_fast16_t size = 0;
    char* start_pointer = page.get_bytes();
    memcpy(&pointer, start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE) + id * (ENTRY_SIZE), POINTER_SIZE);
    memcpy(&size, start_pointer + (N_PAGE_SIZE + DIRSIZE_SIZE + FREESPACE_SIZE + POINTER_SIZE) + id * ENTRY_SIZE, SIZE_SIZE);
    std::vector<uint_fast64_t> tuple = std::vector<uint_fast64_t> (size / ID_SIZE);
    memcpy(&tuple[0], start_pointer + pointer, size);
    return tuple;
}

void SortPage::swap(int x, int y) {
    std::vector<uint_fast64_t> x_tuple = get(x);
    std::vector<uint_fast64_t> y_tuple = get(y);
    override_tuple(x_tuple, y);
    override_tuple(y_tuple, x);
}

void SortPage::quicksort(int i, int f, bool (*is_leq)(std::vector<uint_fast64_t> x, std::vector<uint_fast64_t> y)) {
    if (i <= f) {
        int p = partition(i, f, is_leq);
        quicksort(i, p - 1, is_leq);
        quicksort(p + 1, f, is_leq);
    }
}

int SortPage::partition(int i, int f, bool (*is_leq)(std::vector<uint_fast64_t> x, std::vector<uint_fast64_t> y)) {
    int x = i + (rand() % (f - i + 1));
    std::vector<uint_fast64_t> p = get(x);
    SortPage::swap(x,f);
    override_tuple(p, f);
    int j = i;
    int k = i;
    while(k <= f) {
        if(is_leq(get(k), p)) {
         SortPage::swap(j, k);
        }
        k++;
    }
    SortPage::swap(j, f);
    return j;
}
