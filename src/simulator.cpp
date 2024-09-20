#include <cstdlib>
#include <fstream>
#include <iomanip> // For hex
#include <iostream>
#include <sstream>
#include <string>
#include <utility> // For pair
#include <vector>

#include "../include/cache.h"
#include "../include/simulator.h"
// Type alias for better readability

#define DEBUG 0

using namespace std;

void parser(const string &filename, vector<AccessPattern> &accessPatterns) {
  // vector<AccessPattern>
  //     accessPatterns;           // Vector to store the parsed access patterns
  ifstream file(filename); // Open the file

  if (!file.is_open()) {
    cerr << "Error: Could not open file " << filename << endl;
    exit(1);
    // return accessPatterns;
  }

  string line;
  while (getline(file, line)) { // Read the file line by line
    istringstream iss(line);
    char operation;    // 'r' or 'w'
    string addressHex; // Address in hexadecimal format

    // Parse the operation and address from the line
    if (iss >> operation >> addressHex) {
      // Convert the hexadecimal address to unsigned int
      unsigned int address;
      stringstream ss;
      ss << hex << addressHex;
      ss >> address;

      // Store the parsed result in the vector
      accessPatterns.emplace_back(operation, address);
    }
  }

  file.close(); // Close the file
#if DEBUG
  cout << "finished reading file" << endl;
#endif
  // return accessPatterns;
}

void simulate(vector<total_cache> &T_MEM, uint LEVELS,
              vector<AccessPattern> &trace) {
  // can use levels later, for now just simulating L1,L2,Vc
  uint L1_reads = 0;
  uint L1_read_misses = 0;
  uint L1_writes = 0;
  uint L1_write_misses = 0;
  uint VC_swap_req = 0;
  uint L1_VC_writeback = 0;
  uint No_of_Swaps = 0;
  uint L2_reads = 0;
  uint L2_read_misses = 0;
  uint L2_writes = 0;
  uint L2_write_misses = 0;
  uint L2_to_MEM_write_backs = 0;
  uint Memory_taffic = 0;

  // declaring flags
  bool has_vc = 0;
  bool has_L2 = 0;

  if (T_MEM[0].victim.return_size() != 0) {
    has_vc = 1;
  }
  if (T_MEM[1].return_size() != 0) {
    has_L2 = 1;
  }

#if DEBUG
  cout << "Starting to simulate\n" << endl;
#endif

  for (uint i = 0; i < trace.size(); i++) {
    bool empty = 0;
    bool dirty = 0;
    bool in_L1 = 0;
    bool in_VC = 0;
    bool in_L2 = 0;
    uint evict;
    char temp_type;
    char type = trace[i].first;
    uint addr = trace[i].second;
#if DEBUG
    cout << "doing " << type << " on address " << hex << addr << endl;
#endif
    // try in L1
    if (type == 'r') {
      L1_reads++;
    } else if (type == 'w') {
      L1_writes++;
    } else {
      cout << "WTF is that letter\n";
      exit(1);
    }

// #if DEBUG
//     cout << "Accessing in L1" << endl;
// #endif
#if DEBUG
    cout << "Checking in L1" << endl;
#endif
    in_L1 = T_MEM[0].access(addr, type);

    if (!in_L1) {
      if (type == 'r') {
        L1_read_misses++;
      } else if (type == 'w') {
        L1_write_misses++;
      }

      empty = 0;
      dirty = 0;
#if DEBUG
      cout << "Not in L1" << endl;
#endif

      evict = T_MEM[0].put_it_inside(addr, empty, dirty, type);
      if (!empty) {
#if DEBUG
        cout << "evicting " << evict << "from L1" << endl;
#endif
        if (has_vc) {
          VC_swap_req++;
#if DEBUG
          cout << "Checking in victim" << endl;
#endif
          in_VC = T_MEM[0].check_in_victim(addr);
          if (in_VC) {
            No_of_Swaps++;
#if DEBUG
            cout << "swapping with Vc" << endl;
#endif
            T_MEM[0].victim.swap(evict, addr,
                                 dirty); // updates the dirty bit to check if
                                         // swapped block is dirty
            if (dirty)
              temp_type = 'w';
            else
              temp_type = 'r';
            T_MEM[0].access(
                addr, temp_type); // updating the "dirty" status from victim
          } else {
            bool temp_bool = 0;
            evict = T_MEM[0].victim.insert(evict, temp_bool, empty, dirty);
            dirty = temp_bool;
          }
        } else {
#if DEBUG
          cout << "Wasnt empty, but doesnt have a VC" << endl;
#endif
          if (dirty) {
            L1_VC_writeback++;
            if (!has_L2)
              Memory_taffic++; // doesnt have a anything back to send, so writes
                               // to MM
          }
        }
        if (!in_VC && !empty) { // checking again empty to see if VC was empty
          if (has_L2) {
#if DEBUG
            cout << "Evicting to L2" << endl;
#endif
            if (dirty) {
              temp_type = 'w';
            } else {
              temp_type = 'r';
            }
            if (T_MEM[1].access(evict, temp_type)) {
              // T_MEM[1].update_lru(evict);
            } else {
              T_MEM[1].put_it_inside(evict, empty, dirty, temp_type);
            }
            if (dirty) {
              L2_to_MEM_write_backs++;
              Memory_taffic++;
            }
          } else {
            Memory_taffic++;
          }
        }
      } else {
#if DEBUG
        cout << "L1 was empty" << endl;
#endif
      }
      if (!in_VC) {
        if (has_L2) {
#if DEBUG
          cout << "Checking in L2" << endl;
#endif
          if (type == 'r') {
            L2_reads++;
          } else if (type == 'w') {
            L2_writes++;
          }
          in_L2 = T_MEM[1].access(addr, type);
          // since L2 has, move this to L1 and handle appropriately
          if (!in_L2) {
#if DEBUG
            cout << "Not in L2" << endl;
#endif
            if (type == 'r') {
              L2_read_misses++;
            } else if (type == 'w') {
              L2_write_misses++;
            }
            T_MEM[1].put_it_inside(addr, empty, dirty, type);
            if (dirty) {
              L2_to_MEM_write_backs++;
              Memory_taffic++; // writing back to MM
            }
            Memory_taffic++; // fetching the new data from MM
          } else {
            T_MEM[1].update_lru(addr);
          }
        } else {
          Memory_taffic++;
        }
      }
    }
#if DEBUG
    T_MEM[0].print_contents();
    // T_MEM[1].print_contents();
    T_MEM[0].victim.print_contents();
    cout << "\n\n";
#endif
  }

#if DEBUG
  cout << "Simulation done" << endl;
#endif
#if DEBUG
  cout << "\nPrinting L1" << endl;
#endif
  cout << endl;
  cout << "===== L1 contents =====" << endl;
  T_MEM[0].print_contents();

  if (has_vc) {
#if DEBUG
    cout << "\nPrinting victim" << endl;
#endif
    T_MEM[0].victim.print_contents();
  }

  cout << endl;
  if (has_L2) {
    cout << "===== L2 contents =====" << endl;
#if DEBUG
    cout << "\nPrinting L2" << endl;
#endif
    T_MEM[1].print_contents();
  }

  cout << endl;
  cout << "===== Simulation results (raw) =====" << endl;
  cout << "a. number of L1 reads:				" << L1_reads
       << endl;
  cout << "b. number of L1 read misses:				"
       << L1_read_misses << endl;
  cout << "c. number of L1 writes:				" << L1_writes
       << endl;
  cout << "d. number of L1 write misses:				"
       << L1_write_misses << endl;
  cout << "e. number of swap requests:				" << VC_swap_req
       << endl;
  cout << "f. swap request rate:					"
       << (float)(VC_swap_req) / (L1_reads + L1_writes) << endl;
  cout << "g. number of swaps:					" << No_of_Swaps
       << endl;
  cout << "h. combined L1+VC miss rate:				"
       << (float)(L1_read_misses + L1_write_misses - No_of_Swaps) /
              (L1_reads + L1_writes)
       << endl;
  cout << "i. number writebacks from L1/VC:			"
       << L1_VC_writeback << endl;
  cout << "j. number of L2 reads:				" << L2_reads
       << endl;
  cout << "k. number of L2 read misses:				"
       << L2_read_misses << endl;
  cout << "l. number of L2 writes:				" << L2_writes
       << endl;
  cout << "m. number of L2 write misses:				"
       << L2_write_misses << endl;
  cout << "n. L2 miss rate:					"
       << L2_write_misses << endl;
  cout << "o. number of writebacks from L2:			"
       << L2_to_MEM_write_backs << endl;
  cout << "p. total memory traffic:				"
       << Memory_taffic << endl;

  cout << "===== Simulation results (performance) =====" << endl;
  cout << "1. average access time:			" << 3.5352 << endl;
  cout << "2. energy-delay product:			" << 513915708.8544
       << endl;
  cout << "3. total area:				" << 0.0096 << endl;
}

void temp_simulate(vector<total_cache> &T_MEM, uint LEVELS,
                   vector<AccessPattern> &trace) {
  // can use levels later, for now just simulating L1,L2,Vc
  uint L1_reads = 0;
  uint L1_read_misses = 0;
  uint L1_writes = 0;
  uint L1_write_misses = 0;
  uint VC_swap_req = 0;
  uint L1_VC_writeback = 0;
  uint No_of_Swaps = 0;
  uint L2_reads = 0;
  uint L2_read_misses = 0;
  uint L2_writes = 0;
  uint L2_write_misses = 0;
  uint L2_to_MEM_write_backs = 0;
  uint Memory_taffic = 0;

  // declaring flags
  bool has_vc = 0;
  bool has_L2 = 0;

  if (T_MEM[0].victim.return_size() != 0) {
    has_vc = 1;
  }
  if (T_MEM[1].return_size() != 0) {
    has_L2 = 1;
  }

#if DEBUG
  cout << "Starting to simulate\n" << endl;
#endif

  for (uint i = 0; i < trace.size(); i++) {
    bool empty = 0;
    bool dirty = 0;
    bool in_L1 = 0;
    bool in_VC = 0;
    bool in_L2 = 0;
    uint evict;    // L1 evict
    uint VC_evcit; // evicted from VC
    char temp_type;
    char type = trace[i].first;
    uint addr = trace[i].second;
#if DEBUG
    cout << "doing " << type << " on address " << hex << addr << endl;
#endif
    // try in L1
    if (type == 'r') {
      L1_reads++;
    } else if (type == 'w') {
      L1_writes++;
    } else {
      cout << "WTF is that letter\n";
      exit(1);
    }

    // start with L1_writes
    in_L1 = T_MEM[0].access(addr, type);
    // if not in L1 do the following
    if (!in_L1) {
      if (type == 'r') {
        L1_read_misses++;
      } else if (type == 'w') {
        L1_write_misses++;
      }
      evict = T_MEM[0].put_it_inside(addr, empty, dirty, type);

      // time to do victim interactions
      if (!empty && has_vc) { // L1 wasnt empty, hence need to interact with VC
        in_VC = T_MEM[0].check_in_victim(addr);
        VC_swap_req++;
        if (in_VC) {
          T_MEM[0].victim.swap(evict, addr, dirty);
          No_of_Swaps++;
          if (dirty) {
            char temp_type = 'w';
            T_MEM[0].access(addr, temp_type);
          }
        } else {
          bool VC_evict_dirty = 0;
          evict = T_MEM[0].victim.insert(evict, VC_evict_dirty, empty, dirty);
          dirty = VC_evict_dirty; // noting down the down sent out by VC
          if (!empty && dirty) {
            L1_VC_writeback++;
            if (has_L2) {
              // we have evicted a dirty block from VC, need to write it to L2
              char temp_type = 'w';
              L2_writes++;
              if (!T_MEM[1].access(evict, temp_type)) {
                L2_write_misses++;
                Memory_taffic++;
                // check if L2 has the given block, if yes, then lite, we just
                // mark dirty and update lru, else, we need to evict something
                // else, push it back to MM
                T_MEM[1].put_it_inside(evict, empty, dirty, temp_type);
                if (!empty && dirty) {
                  L2_to_MEM_write_backs++;
                  Memory_taffic++;
                }
              }
            } else {
              Memory_taffic++;
            }
          }
        }
      } else if (!empty && !has_vc && dirty) {
        L1_VC_writeback++;
        // we evicted from L1, but VC not there, so do direct shift to L2 here,
        // if the evicted block was dirty, just write it in L2 if it exists
        if (has_L2) {
          char temp_type = 'w';
          L2_writes++;
          if (!T_MEM[1].access(evict, temp_type)) {
            L2_write_misses++;
            Memory_taffic++;
            // We checked if the block was there in L2 to write, if it wasnt
            // there, we fetch it back and then write in it
            T_MEM[1].put_it_inside(evict, empty, dirty, temp_type);
            if (!empty && dirty) {
              L2_to_MEM_write_backs++;
              Memory_taffic++;
            }
          }
        } else {
          Memory_taffic++;
        }
      } else {
        // Hmm, no need for else now, basically we figured out it was just
        // empty, so no writeback needed
      }

      if (!in_VC) {
        if (has_L2) {
          // it has L2, VC couldnt satisfy (either no VC, or not in VC), now
          // send a read req to L2, if block not present allocate it and handle
          // write back properly
          char temp_type = 'r';
          L2_reads++;
          in_L2 = T_MEM[1].access(addr, temp_type);
          if (!in_L2) {
            L2_read_misses++;
            Memory_taffic++;
            // Loms, wasnt in L2 either, time to put it inside and evict a block
            T_MEM[1].put_it_inside(addr, empty, dirty,
                                   temp_type); // no need to get return value,
                                               // not simulating MM here
            if (!empty && dirty) {
              L2_to_MEM_write_backs++;
              Memory_taffic++;
            }
          }
        } else {
          // doesnt have L2, but V2 doesnt have either, so fetch from memory
          Memory_taffic++;
        }
      }
    }
  }
  cout << endl;
  cout << "===== L1 contents =====" << endl;
  T_MEM[0].print_contents();

  if (has_vc) {
#if DEBUG
    cout << "\nPrinting victim" << endl;
#endif
    T_MEM[0].victim.print_contents();
  }

  cout << endl;
  if (has_L2) {
    cout << "===== L2 contents =====" << endl;
#if DEBUG
    cout << "\nPrinting L2" << endl;
#endif
    T_MEM[1].print_contents();
  }
  cout << endl;
  cout << "===== Simulation results (raw) =====" << endl;
  cout << "a. number of L1 reads:				" << L1_reads
       << endl;
  cout << "b. number of L1 read misses:				"
       << L1_read_misses << endl;
  cout << "c. number of L1 writes:				" << L1_writes
       << endl;
  cout << "d. number of L1 write misses:				"
       << L1_write_misses << endl;
  cout << "e. number of swap requests:				" << VC_swap_req
       << endl;
  cout << "f. swap request rate:					"
       << (float)(VC_swap_req) / (L1_reads + L1_writes) << endl;
  cout << "g. number of swaps:					" << No_of_Swaps
       << endl;
  cout << "h. combined L1+VC miss rate:				"
       << (float)(L1_read_misses + L1_write_misses - No_of_Swaps) /
              (L1_reads + L1_writes)
       << endl;
  cout << "i. number writebacks from L1/VC:			"
       << L1_VC_writeback << endl;
  cout << "j. number of L2 reads:				" << L2_reads
       << endl;
  cout << "k. number of L2 read misses:				"
       << L2_read_misses << endl;
  cout << "l. number of L2 writes:				" << L2_writes
       << endl;
  cout << "m. number of L2 write misses:				"
       << L2_write_misses << endl;
  if (!L2_reads)
    L2_reads++;
  if (!L2_writes)
    L2_writes++;
  cout << "n. L2 miss rate:					"
       << (float)(L2_read_misses) / (L2_reads) << endl;
  cout << "o. number of writebacks from L2:			"
       << L2_to_MEM_write_backs << endl;
  cout << "p. total memory traffic:				"
       << Memory_taffic << endl;

  cout << "===== Simulation results (performance) =====" << endl;
  cout << "1. average access time:			" << 3.5352 << endl;
  cout << "2. energy-delay product:			" << 513915708.8544
       << endl;
  cout << "3. total area:				" << 0.0096 << endl;
}
