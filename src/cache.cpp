#include "../include/cache.h"
#include <cstring>
#include <vector>

using namespace std;

int find_lru(total_cache &A) { // use it to find the set to insert it into
  //
}

base_cache::base_cache(unsigned int block_size, unsigned int num_lines) {
  this->block_size = block_size;
  this->num_lines = num_lines;
  this->tag_array.resize(num_lines);
  this->valid_array.resize(num_lines);
  this->lru_arr.resize(num_lines);
  this->ditry.resize(num_lines);

  for (uint i; i < num_lines; i++) {
    this->valid_array[i] = -1;
    this->ditry[i] = 0;
  }
}

int base_cache::read(uint &address) {
  uint C_line = address / (this->block_size);
  uint line = C_line % (this->num_lines);
  if (valid_array[line])
    if (tag_array[line] == C_line / line)
      return 1;
  return 0;
}

int base_cache::write(uint &address) {
  uint C_line = address / (this->block_size);
  uint line = C_line % (this->num_lines);
  if (valid_array[line])
    if (tag_array[line] == C_line / line) {
      this->ditry[line] = 1;
      return 1;
    }
  return 0;
}

total_cache::total_cache(unsigned int assoc, unsigned int block_size,
                         unsigned int total_size) {
  this->assoc = assoc;
  for (uint i = 0; i < assoc; i++) {
    this->banks.emplace_back(block_size, total_size / (block_size * assoc));
  }
}

int total_cache::read(uint &address) {
  for (uint i = 0; i < this->assoc; i++) {
    if (this->banks[i].read(address)) {
      return 1;
    }
  }
  return 0;
}

int total_cache::write(uint &address) { // should implement
  for (uint i = 0; i < this->assoc; i++) {
    if (this->banks[i].write(address)) {
      return 1;
    }
  }
  return 0;
}
