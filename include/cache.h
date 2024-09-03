#pragma once
#include <vector>

using namespace std;

using uint = unsigned int;

#define ADDRESS_BITS 32

class Memory { // useless
private:
  unsigned int block_size;
  unsigned int num_lines;
  unsigned char data;
};

class base_cache { // one assoc of cache, no memory in simulator as not
                   // important
private:
  unsigned int block_size;
  unsigned int num_lines;
  vector<uint> tag_array;
  vector<uint> valid_array;
  vector<uint> lru_arr;
  vector<uint> ditry;

public:
  base_cache(unsigned int block_size, unsigned int num_lines);
  int read(uint &address);
  int write(uint &address);
  void insert(uint &address);
  ~base_cache();
};

class total_cache { // culmination of all base caches
private:
  unsigned int assoc;
  std::vector<base_cache> banks;

public:
  total_cache(unsigned int assoc, unsigned int block_size,
              unsigned int total_size);
  int read(uint &address);  // return 1 if read successful i.e it was a hit
  int write(uint &address); // return 1 if write successful i.e it was a hit
  void insert(uint &address);
  ~total_cache();
};

class multi_level_cache { // can use for multiple levels of cache
  //
};
