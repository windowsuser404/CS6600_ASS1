#include "../include/cache.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
#include <strings.h>
#include <vector>

using namespace std;

base_cache::base_cache(unsigned int block_size, unsigned int num_lines)
    : block_size(block_size), num_lines(num_lines) {
  this->tag_array.resize(num_lines);
  this->valid_array.resize(num_lines);
  this->ditry.resize(num_lines);

  for (uint i = 0; i < num_lines; i++) {
    this->valid_array[i] = 0;
    this->ditry[i] = 0;
  }
}

int base_cache::access(uint &line, char &type, uint &tag) {
  if (valid_array[line])
    if (tag_array[line] == tag) {
      if (type == 'w')
        this->ditry[line] = 1;
      return 1;
    }
  return 0;
}

victim_cache::victim_cache(uint block_size, uint num_lines)
    : base_cache(block_size, num_lines) {
  this->valid_array.resize(num_lines);
}

uint victim_cache::find_lru() {
  int index = -1;
  int max = -1;
  for (uint i = 0; i < this->num_lines; i++) {
    if (max < this->lru_array[i]) {
      index = i;
      max = this->lru_array[i];
    }
  }
  return (uint)index;
}

void victim_cache::update_lru(uint &address) {
  uint line = address % (this->block_size * this->num_lines);
  uint tag = address / (this->block_size * this->num_lines);
  uint temp_lru;
  uint temp_bank;
  for (uint i = 0; i < this->num_lines; i++) {
    if (tag == this->tag_array[i]) {
      temp_lru = this->lru_array[i];
      temp_bank = i;
    }
  }
  for (uint i = 0; i < this->num_lines; i++) {
    if (lru_array[i] < temp_lru) {
      lru_array[i]++;
    }
  }
  lru_array[temp_bank] = 0;
}

uint victim_cache::insert(uint &address, int &is_dirty) {
  uint evict;
  for (uint i = 0; i < num_lines; i++) {
    if (tag_array[i] < num_lines - 1) {
      lru_array[i]++;
    } else {
      is_dirty = this->ditry[i];
      evict = tag_array[i] * this->block_size;
      lru_array[i] = 0;
      tag_array[i] = address / block_size;
    }
  }
  return evict;
}

total_cache::total_cache(uint assoc, uint block_size, uint total_size,
                         uint number_of_victims)
    : victim(block_size, number_of_victims), assoc(assoc),
      block_size(block_size), total_size(total_size) {
  this->lines_per_bank = total_size / (block_size * assoc);
  this->lru_arr.resize(assoc);
  for (uint i = 0; i < assoc; i++) {
    this->banks.emplace_back(block_size, lines_per_bank);
    this->lru_arr.resize(lines_per_bank);
  }
}

int total_cache::access(uint &address, char &type) {
  int found = 0;
  int temp_lru = -1;
  uint i;
  uint line = line_generator(address);
  uint tag = tag_generator(address);
  for (i = 0; i < this->assoc; i++) {
    found = this->banks[i].access(line, type, tag);
    if (found) {
      temp_lru = this->lru_arr[i][line];
      break;
    }
  }
  if (found) {
    for (uint j = 0; j < this->assoc; j++) {
      if (this->lru_arr[j][line] < temp_lru)
        this->lru_arr[j][line]++;
    }
    this->lru_arr[i][line] = 0;
    return 1;
  } else
    return 0;
}

uint total_cache::find_lru(
    uint &address) { // use it to find the set to insert it into
  uint line_number = address % (this->lines_per_bank);
  uint max = this->lru_arr[0][line_number];
  uint index = 0;
  for (uint i = 0; i < this->assoc; i++) {
    if (this->lru_arr[assoc][line_number] > max) {
      index = i;
      max = this->lru_arr[assoc][line_number];
    }
  }
  return index;
}

void total_cache::update_lru(uint &address) {
  uint line = line_generator(address);
  uint tag = tag_generator(address);
  uint temp_lru;
  uint temp_bank;
  for (uint i = 0; i < banks.size(); i++) {
    if (tag == banks[i].tag_array[line]) {
      temp_lru = lru_arr[i][line];
      temp_bank = i;
    }
  }
  for (uint i = 0; i < banks.size(); i++) {
    if (lru_arr[i][line] < temp_lru) {
      lru_arr[i][line]++;
    }
  }
  lru_arr[temp_bank][line] = 0;
}

uint total_cache::insert(uint &address, uint &line, uint &tag, int &dirty) {
  uint index = this->find_lru(address);
  // to get back the original cache line number
  uint evicted_address = this->banks[index].tag_array[line] *
                         this->lines_per_bank * this->block_size;
  dirty = this->banks[index].ditry[line];
  this->banks[index].tag_array[line] = tag;
  this->banks[index].ditry[line] = 0;
  update_lru(address);
  return evicted_address;
}

uint total_cache::put_it_inside(uint &address, bool &empty, int &dirty) {
  uint line = line_generator(address);
  uint tag = tag_generator(address);

  uint index;
  uint evict = 0;
  dirty = 0;
  empty = false;
  for (uint i = 0; i < assoc; i++) {
    if (!banks[i].valid_array[line]) {
      index = i;
      empty = true;
      break;
    }
  }
  if (empty) {
    this->banks[index].tag_array[line] = tag;
    this->banks[index].valid_array[line] = 1;
  } else {
    uint evict = this->insert(address, line, tag, dirty);
  }
  return evict;
}

int total_cache::check_in_victim(uint &address) {
  int index = -1;
  uint temp_lru;
  uint evicted_address;
  for (uint i = 0; i < this->victim.num_lines; i++) {
    if (this->victim.tag_array[i] == address / this->victim.num_lines &&
        this->victim.valid_array[i]) {
      temp_lru = victim.lru_array[i];
      index = i;
      bool temp1;
      int temp2;
      evicted_address = this->put_it_inside(address, temp1, temp2);
      break;
    }
  }
  if (index == -1)
    return 0;
  else {
    this->victim.tag_array[index] =
        evicted_address / this->block_size; // fully associative
    for (uint i = 0; i < this->victim.num_lines; i++) {
      if (victim.lru_array[i] < temp_lru) {
        victim.lru_array[i]++;
      }
    }
    victim.lru_array[index] = 0;
    return 1;
  }
}

uint total_cache::line_generator(uint &address) {
  return address % (this->block_size * this->lines_per_bank);
}

uint total_cache::tag_generator(uint &address) {
  return address / (this->block_size * this->lines_per_bank);
}
