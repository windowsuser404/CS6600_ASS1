#pragma once

// all memory related stuff

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

class total_cache;

class base_cache { // one assoc of cache, no memory in simulator as not
                   // important
protected:
  uint block_size;
  uint num_lines;
  vector<uint> tag_array;
  vector<uint> valid_array;
  vector<uint> ditry;

public:
  base_cache(uint block_size, uint num_lines);
  int access(uint &line, char &type, uint &tag);
  virtual uint insert(uint &address) {
    // can be redefined if needed later
    return 1;
  }
  friend total_cache;
};

class victim_cache : protected base_cache {
public:
  void swap(uint &to_insert, uint &to_remove, bool &dirty);
  victim_cache(uint block_size, uint num_lines);
  void print_contents();
  uint find_lru();
  void update_lru(uint &address);
  uint insert(uint &address, bool &dirty, bool &empty, bool was_dirty);
  vector<uint> lru_array;
  uint is_dirty(uint &i);
  uint return_size();
  friend total_cache;
};

class total_cache { // culmination of all base caches
private:
  uint assoc;
  uint lines_per_bank;
  vector<vector<uint>>
      lru_arr; // outer is to select bank, inner is to select line
  uint block_size;
  uint total_size;
  vector<base_cache> banks;
  uint line_generator(uint &address);
  uint tag_generator(uint &address);
  uint
  insert(uint &address, uint &line, uint &tag, bool &dirty,
         char &type); // returns the popped off values, can be ignored
                      // if not needed, assuming it gives back the full address

public:
  victim_cache victim;
  total_cache(uint assoc, uint block_size, uint total_size,
              uint number_of_victims);
  int access(uint &address, char &type);
  uint find_lru(uint &address); // use it to find the set to insert it into
  void update_lru(uint &address);
  int check_in_victim(uint &address);
  uint put_it_inside(uint &address, bool &empty, bool &dirty, char &type);
  void print_contents();
  uint return_size();
};
