#include "../include/cache.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <strings.h>
#include <vector>

#define DEBUG 1

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
  this->lru_array.resize(num_lines);
}

uint victim_cache::find_lru() {
  uint index = 0;
  uint max = this->lru_array[0];
  for (uint i = 0; i < this->num_lines; i++) {
    if (max < this->lru_array[i]) {
      index = i;
      max = this->lru_array[i];
    }
  }
  return (uint)index;
}

void victim_cache::swap(uint &to_insert, uint &to_remove, bool &dirty) {
  uint tag = to_remove / block_size;
  update_lru(to_remove);
#if DEBUG
  cout << "Going to remove " << to_remove << " From VC" << endl;
#endif
  for (uint i = 0; i < num_lines; i++) {
    if (tag_array[i] == tag) {
      tag_array[i] = to_insert / block_size;
      this->ditry[i] = dirty;
      break;
    }
  }
}

void victim_cache::update_lru(uint &address) {
  // uint line = address % (this->block_size * this->num_lines);
  uint tag = address / (this->block_size * this->num_lines);
  uint temp_lru = 0;
  uint temp_bank = 0;
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

uint victim_cache::insert(uint &address, bool &is_dirty, bool &empty,
                          bool was_dirty) {
  uint evict = 0;
  is_dirty = 0;
  empty = 0;
  // #if DEBUG
  //   cout << "Inserting " << address << " in VC" << endl;
  // #endif
  for (uint i = 0; i < num_lines; i++) {
#if DEBUG
    cout << "doing " << i << "th line" << endl;
#endif
    if (valid_array[i]) {
#if DEBUG
      cout << i << "th line is valid with lru " << lru_array[i] << endl;
#endif
      if (lru_array[i] < num_lines - 1) {
        // #if DEBUG
        //         cout << "increasing lru of " << i << "th line" << endl;
        // #endif
        lru_array[i]++;
      } else {
#if DEBUG
        cout << "found lru in " << i << "th line" << endl;
#endif
        is_dirty = this->ditry[i];
        evict = tag_array[i] * block_size;
        lru_array[i] = 0;
        tag_array[i] = address / block_size;
        ditry[i] = was_dirty;
      }
    } else {
#if DEBUG
      cout << "VC has empty " << i << " th line" << endl;
#endif
      empty = 1;
      lru_array[i] = 0;
      tag_array[i] = address / block_size;
      valid_array[i] = 1;
      ditry[i] = was_dirty;
      break;
    }
  }
  return evict;
}

uint victim_cache::return_size() { return num_lines * block_size; }

total_cache::total_cache(uint assoc, uint block_size, uint total_size,
                         uint number_of_victims)
    : assoc(assoc), block_size(block_size), total_size(total_size),
      victim(block_size, number_of_victims) {
  if (total_size != 0) {
    this->lines_per_bank = total_size / (block_size * assoc);
    this->lru_arr.resize(assoc);
    for (uint i = 0; i < assoc; i++) {
      this->banks.emplace_back(block_size, lines_per_bank);
      this->lru_arr[i].resize(lines_per_bank);
    }
  }
}

int total_cache::access(uint &address, char &type) {
  uint found = 0;
  uint temp_lru = 0;
  uint i;
  uint line = line_generator(address);
  uint tag = tag_generator(address);

#if DEBUG
  cout << "checking in " << line << " for tag " << tag << endl;
#endif

  for (i = 0; i < this->assoc; ++i) {
    found = this->banks[i].access(line, type, tag);
    if (found) {
      temp_lru = this->lru_arr[i][line];
#if DEBUG
      cout << "found in bank " << i << " with lru as " << temp_lru << endl;
#endif
      break;
    }
  }
  if (found) {
    for (uint j = 0; j < this->assoc; j++) {
      if (this->lru_arr[j][line] < temp_lru) {
        // #if DEBUG
        //         cout << "increasing lru of bank " << j << endl;
        // #endif
        this->lru_arr[j][line]++;
      }
    }
#if DEBUG
    cout << "making bank " << i << " line " << line << " as lru" << endl;
#endif
    this->lru_arr[i][line] = 0;
#if DEBUG
    cout << "found in cache" << endl;
#endif
    return 1;
  } else {
#if DEBUG
    cout << "not found in cache" << endl;
#endif
    return 0;
  }
}

uint total_cache::find_lru(
    uint &address) { // use it to find the set to insert it into
                     // #if DEBUG
                     //   cout << "Finding the LRU bank to replace" << endl;
                     // #endif
  uint line_number = line_generator(address);
  // #if DEBUG
  //   cout << "line number " << line_number << " in cache" << endl;
  // #endif
  // #if DEBUG
  //   cout << lru_arr.size() << " " << lru_arr[0].size() << endl;
  // #endif
  uint max = this->lru_arr[0][line_number];
  uint index = 0;
  for (uint i = 0; i < this->assoc; i++) {
#if DEBUG
    // cout << "bank " << i << " line " << line_number << "has lru "
    //      << lru_arr[i][line_number] << endl;
#endif
    if (this->lru_arr[i][line_number] > max) {
      index = i;
      max = this->lru_arr[i][line_number];
    }
  }
#if DEBUG
  cout << "found bank " << index << " as lru" << endl;
#endif
  return index;
}

void total_cache::update_lru(uint &address) {
  uint line = line_generator(address);
  uint tag = tag_generator(address);
#if DEBUG
  cout << "updating lru of line " << line << endl;
#endif
  uint temp_lru = 0;
  uint temp_bank = 0;
  for (uint i = 0; i < banks.size(); i++) {
    if (tag == banks[i].tag_array[line] && banks[i].valid_array[line]) {
      temp_lru = lru_arr[i][line];
      temp_bank = i;
    }
  }
#if DEBUG
  cout << "Current lru " << temp_lru << endl;
#endif
  for (uint i = 0; i < banks.size(); i++) {
#if DEBUG
    // cout << "bank " << i << " has line " << line << " lru " <<
    // lru_arr[i][line]
    //      << endl;
#endif
    if (lru_arr[i][line] < temp_lru && banks[i].valid_array[line]) {
      lru_arr[i][line]++;
    }
  }
  lru_arr[temp_bank][line] = 0;
}

uint total_cache::insert(uint &address, uint &line, uint &tag, bool &dirty,
                         char &type) {

  uint index = this->find_lru(address);

#if DEBUG
  cout << "Inserting address " << address << " in line " << line << " of bank "
       << index << endl;
#endif
  // to get back the original cache line number
  uint evicted_address =
      (this->banks[index].tag_array[line] * this->lines_per_bank + line) *
      this->block_size;
  dirty = this->banks[index].ditry[line];
  this->banks[index].tag_array[line] = tag;
  if (type == 'w') {
    this->banks[index].ditry[line] = 1;
  } else {
    this->banks[index].ditry[line] = 0;
  }
  update_lru(address);

  return evicted_address;
}

uint total_cache::put_it_inside(uint &address, bool &empty, bool &dirty,
                                char &type) {
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
    for (uint i = 0; i < assoc; i++) {
      if (banks[i].valid_array[line]) {
        lru_arr[i][line]++;
      }
    }
    this->banks[index].tag_array[line] = tag;
    this->banks[index].valid_array[line] = 1;
    if (type == 'w') {
      this->banks[index].ditry[line] = 1;
    } else {
      this->banks[index].ditry[line] = 0;
    }
    lru_arr[index][line] = 0;
  } else {
    evict = this->insert(address, line, tag, dirty, type);
  }
  return evict;
}

int total_cache::check_in_victim(uint &address) {
#if DEBUG
  cout << "checking for " << address << endl;
#endif
  int index = -1;
  uint temp_lru;
  uint temp_dirty;
  char temp_type;
  uint evicted_address;
  for (uint i = 0; i < this->victim.num_lines; i++) {
    if (this->victim.tag_array[i] == address / this->block_size &&
        this->victim.valid_array[i]) {
      temp_lru = victim.lru_array[i];
      temp_dirty = victim.ditry[i];
      index = i;
      // bool temp1;
      // bool temp2;
      // if (temp_dirty)
      //   temp_type = 'w';
      // else
      //   temp_type = 'r';
      // evicted_address = this->put_it_inside(address, temp1, temp2,
      // temp_type);
      break;
    }
  }
  if (index == -1) {
#if DEBUG
    cout << "Not in VC" << endl;
#endif
    return 0;
  } else {
    // this->victim.tag_array[index] =
    //     evicted_address / this->block_size; // fully associative
    // for (uint i = 0; i < this->victim.num_lines; i++) {
    //   if (victim.lru_array[i] < temp_lru) {
    //     victim.lru_array[i]++;
    //   }
    // }
    // victim.lru_array[index] = 0;
#if DEBUG
    cout << "in VC" << endl;
#endif
    return 1;
  }
}

uint total_cache::line_generator(uint &address) {
  // #if DEBUG
  //   cout << "address=" << address
  //        << " line=" << address % (this->block_size * this->lines_per_bank)
  //        << endl;
  //   cout << "block size " << this->block_size << " lines_per_bank "
  //        << lines_per_bank << endl;
  // #endif
  return (address / (this->block_size)) % this->lines_per_bank;
}

uint total_cache::tag_generator(uint &address) {
  // #if DEBUG
  //   cout << "address=" << address
  //        << " tag=" << address / (this->block_size * this->lines_per_bank)
  //        << endl;
  // #endif
  return address / (this->block_size * this->lines_per_bank);
}

uint total_cache::return_size() { return this->total_size; }

//
void total_cache::print_contents() {
  // make it MRU later
  for (uint i = 0; i < banks.size(); i++) {
    cout << "Bank " << i << " contents" << endl;
    for (uint j = 0; j < banks[i].tag_array.size(); j++) {
      cout << banks[i].tag_array[j] * (lines_per_bank) + j;
      cout << " V=" << banks[i].valid_array[j];
      cout << " D=" << banks[i].ditry[j] << endl;
    }
  }
}

void victim_cache::print_contents() {
  // make it MRU later
  cout << "Printing Victim Cache" << endl;
  for (uint i = 0; i < num_lines; i++) {
    cout << tag_array[i];
    cout << " V=" << valid_array[i];
    cout << " D=" << ditry[i] << endl;
  }
}
