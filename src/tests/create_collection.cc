#include "storage/tuple_collection/tuple_collection.h"

#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include <chrono>
#include <climits>
#include <vector>
#include <iostream>

using namespace std;



void print(std::vector<unsigned char> vec)
{
  for (unsigned char c : vec)
  {
    std::cout << c;
  }
  std::cout << endl;
}

bool compare(std::vector<uint64_t> x, std::vector<uint64_t> y)
{
  return x[0] < y[0];
}

std::vector<uint_fast64_t> v1 = std::vector<uint_fast64_t>{3, 1, 2};
std::vector<uint_fast64_t> v2 = std::vector<uint_fast64_t>{1, 3, 2};
std::vector<uint_fast64_t> v3 = std::vector<uint_fast64_t>{2, 1, 3};

int main()
{
  FileManager::init(".");
  BufferManager::init(BufferManager::DEFAULT_BUFFER_POOL_SIZE);
  FileId file_id = file_manager.get_file_id("tuple_collection_example.txt");
  TupleCollection tuple_collection(buffer_manager.append_page(file_id), v1.size());
  tuple_collection.add(v1);
  tuple_collection.add(v2);
  tuple_collection.add(v3);
  tuple_collection.sort(&compare);
  std::vector<uint_fast64_t> u1 = tuple_collection.get(0);
  std::vector<uint_fast64_t> u2 = tuple_collection.get(1);
  std::vector<uint_fast64_t> u3 = tuple_collection.get(2);
  cout << u1[0] << "," << u1[1] << "," << u1[2] << "\n";
  cout << u2[0] << "," << u2[1] << "," << u2[2] << "\n";
  cout << u3[0] << "," << u3[1] << "," << u3[2] << "\n";
  buffer_manager.~BufferManager();
  file_manager.~FileManager();
  return 0;
}
