#include "../include/cache.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <strings.h>
#include <vector>

using namespace std;

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
  this->total_size = total_size;
  this->block_size = block_size;
  this->lines_per_bank = total_size / (block_size * assoc);
  for (uint i = 0; i < assoc; i++) {
    this->banks.emplace_back(block_size, lines_per_bank);
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

uint total_cache::find_lru(
    uint &address) { // use it to find the set to insert it into
  int max = -1;
  int index = -1;
  uint line_number = address % (this->lines_per_bank);
  for (uint i = 0; i < this->assoc; i++) {
    if (this->banks[i].lru_arr[line_number] > max) {
      index = i;
      max = this->banks[i].lru_arr[line_number];
    }
  }
  return (uint)index;
}
