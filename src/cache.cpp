#include "../include/cache.h"
#include "../include/cacti_parser.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <strings.h>
#include <vector>

#define DEBUG 0

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

void victim_cache::Put_params() {
  //
  int err = get_cacti_results(this->num_lines * this->block_size,
                              this->block_size, this->num_lines,
                              &this->AccessTime, &this->Energy, &this->Area);
  if (err) {
    cout << "Cacti error be happening in VC" << endl;
    exit(1);
  }
}

victim_cache::victim_cache(uint block_size, uint num_lines)
    : base_cache(block_size, num_lines) {
  this->lru_array.resize(num_lines);
  // cout << "populating victim params" << endl;
  if (num_lines)
    Put_params();
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
  bool temp_dirty;
  update_lru(to_remove);

  for (uint i = 0; i < num_lines; i++) {
    if (tag_array[i] == tag && valid_array[i]) {
      tag_array[i] = to_insert / block_size;
      temp_dirty = this->ditry[i];
      this->ditry[i] = dirty;
      dirty = temp_dirty;
      break;
    }
  }
}

void victim_cache::update_lru(uint &address) {
  uint tag = address / (this->block_size);
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

uint victim_cache::insert(uint &address, bool &VC_evict_dirty, bool &empty,
                          bool &L1_evict_dirty) {
  uint evict = 0;
  VC_evict_dirty = 0;
  empty = 0;
  for (uint i = 0; i < num_lines; i++) {
    if (valid_array[i]) {
      if (lru_array[i] < num_lines - 1) { // cant be the lru
        lru_array[i]++;

      } else {

        VC_evict_dirty = this->ditry[i];
        evict = tag_array[i] * block_size;
        lru_array[i] = 0;
        tag_array[i] = address / block_size;
        ditry[i] = L1_evict_dirty;
      }
    } else {
      empty = 1;
      lru_array[i] = 0;
      tag_array[i] = address / block_size;
      valid_array[i] = 1;
      ditry[i] = L1_evict_dirty;

      break;
    }
  }
  return evict;
}

uint victim_cache::return_size() { return num_lines * block_size; }

void total_cache::Put_params() {
  int err = get_cacti_results(total_size, block_size, assoc, &AccessTime,
                              &Energy, &Area);
  if (err) {
    cout << "cacti throwing error in big cache" << endl;
    exit(1);
  }
}

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
  if (total_size != 0)
    Put_params();
}

int total_cache::access(uint &address, char &type) {
  uint found = 0;
  uint temp_lru = 0;
  uint i;
  uint line = line_generator(address);
  uint tag = tag_generator(address);

  for (i = 0; i < this->assoc; ++i) {
    found = this->banks[i].access(line, type, tag);
    if (found) {
      temp_lru = this->lru_arr[i][line];

      break;
    }
  }
  if (found) {
    for (uint j = 0; j < this->assoc; j++) {
      if (this->lru_arr[j][line] < temp_lru) {
        this->lru_arr[j][line]++;
      }
    }

    this->lru_arr[i][line] = 0;
    return 1;
  } else {
    return 0;
  }
  update_lru(address);
}

uint total_cache::find_lru(
    uint &address) { // use it to find the set to insert it into
  uint line_number = line_generator(address);
  uint max = this->lru_arr[0][line_number];
  uint index = 0;
  for (uint i = 0; i < this->assoc; i++) {
    if (this->lru_arr[i][line_number] > max) {
      index = i;
      max = this->lru_arr[i][line_number];
    }
  }
  return index;
}

void total_cache::update_lru(uint &address) {
  uint line = line_generator(address);
  uint tag = tag_generator(address);

  uint temp_lru = 0;
  uint temp_bank = 0;
  for (uint i = 0; i < banks.size(); i++) {
    if (tag == banks[i].tag_array[line] && banks[i].valid_array[line]) {
      temp_lru = lru_arr[i][line];
      temp_bank = i;
    }
  }

  for (uint i = 0; i < banks.size(); i++) {
    if (lru_arr[i][line] < temp_lru && banks[i].valid_array[line]) {
      lru_arr[i][line]++;
    }
  }
  lru_arr[temp_bank][line] = 0;
}

uint total_cache::insert(uint &address, uint &line, uint &tag, bool &dirty,
                         char &type) {

  uint index = this->find_lru(address);

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

  bool found = 0;
  uint temp_lru;
  uint temp_dirty;
  char temp_type;
  uint evicted_address;
  for (uint i = 0; i < this->victim.num_lines; i++) {
    if (this->victim.tag_array[i] == address / this->block_size &&
        this->victim.valid_array[i]) {
      temp_lru = victim.lru_array[i];
      temp_dirty = victim.ditry[i];
      found = 1;
      break;
    }
  }
  if (!found) {

    return 0;
  } else {
    return 1;
  }
}

uint total_cache::line_generator(uint &address) {
  //
  return (address / (this->block_size)) % this->lines_per_bank;
}

uint total_cache::tag_generator(uint &address) {
  //
  return address / (this->block_size * this->lines_per_bank);
}

uint total_cache::return_size() { return this->total_size; }

//
void total_cache::print_contents() {
  // make it MRU later
  for (uint j = 0; j < banks[0].tag_array.size(); j++) {
    cout << "set " << j << ": ";
    vector<pair<uint, uint>> temp_lru;
    for (uint i = 0; i < banks.size(); i++) {
      temp_lru.push_back({lru_arr[i][j], i});
    }

    //

    std::sort(temp_lru.begin(), temp_lru.end(),
              [](pair<uint, uint> &a, pair<uint, uint> &b) {
                return a.first < b.first;
              });
    for (uint i = 0; i < temp_lru.size(); i++) {

      cout << hex << banks[temp_lru[i].second].tag_array[j];
      cout << dec << " ";
      if (banks[temp_lru[i].second].ditry[j])
        cout << "D ";
      else
        cout << "  ";
    }
    cout << endl;
  }
}

void victim_cache::print_contents() {
  // make it MRU later
  cout << "===== VC contents =====" << endl;
  vector<pair<uint, uint>> temp_lru;
  for (uint i = 0; i < num_lines; i++) {
    temp_lru.push_back({lru_array[i], i});
  }
  std::sort(temp_lru.begin(), temp_lru.end(),
            [](pair<uint, uint> &a, pair<uint, uint> &b) {
              return a.first < b.first;
            });
  cout << "set " << 0 << ":";
  for (uint i = 0; i < num_lines; i++) {

    cout << " " << hex << tag_array[temp_lru[i].second];
    cout << dec << " ";
    if (this->ditry[temp_lru[i].second])
      cout << "D ";
    else
      cout << "  ";
  }
}
