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

#define DEBUG 1

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
  bool empty = 0;
  bool dirty = 0;

  if (T_MEM[0].victim.return_size() != 0) {
    has_vc = 1;
  }
  if (T_MEM[1].return_size() != 0) {
    has_L2 = 1;
  }

  bool in_L1 = 0;
  bool in_VC = 0;
  bool in_L2;
  uint evict;

#if DEBUG
  cout << "Starting to simulate\n" << endl;
#endif

  for (uint i = 0; i < trace.size(); i++) {
    char type = trace[i].first;
    uint addr = trace[i].second;
#if DEBUG
    cout << "doing " << type << " on address " << addr << endl;
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
    if (in_L1) {
      T_MEM[0].update_lru(addr);
    } else {
      if (type == 'r') {
        L1_read_misses++;
      } else if (type == 'w') {
        L1_write_misses++;
      }
      if (has_vc) {
        VC_swap_req++;
        in_VC = T_MEM[0].check_in_victim(addr);
        if (in_VC) {
          No_of_Swaps++;
        }
      }
    }

    if (!in_L1) {

#if DEBUG
      cout << "Not in L1 and VC" << endl;
#endif

      empty = 0;
      dirty = 0;
      evict = T_MEM[0].put_it_inside(addr, empty, dirty, type);
      if (!empty) {
        if (has_vc) {
#if DEBUG
          cout << "inserting " << evict << " into VC" << endl;
#endif
          if (dirty)
            evict = T_MEM[0].victim.insert(evict, dirty, empty, 1);
          else
            evict = T_MEM[0].victim.insert(evict, dirty, empty, 0);
        }
        if (!in_VC) {
          if (dirty && has_L2) {

#if DEBUG
            cout << "Acessing in L2" << endl;
#endif
            // block wass dirty write into L2
#if DEBUG
            cout << "Victim pushin to L2" << endl;
#endif

            T_MEM[1].put_it_inside(evict, empty, dirty, type);
            if (dirty) {
              L2_to_MEM_write_backs++;
            }
          }
        }
        if (has_L2) {
          if (type == 'r') {
            L2_reads++;
          } else if (type == 'w') {
            L2_writes++;
          }
          in_L2 = T_MEM[1].access(addr, type);
          T_MEM[1].update_lru(addr);
          // since L2 has, move this to L1 and handle appropriately
          if (!in_L2) {
            if (type == 'r') {
              L2_read_misses++;
            } else if (type == 'w') {
              L2_write_misses++;
            }
            T_MEM[1].put_it_inside(addr, empty, dirty, type);
            if (dirty) {
              L2_to_MEM_write_backs++;
            }
          }
        }
      }
    }
#if DEBUG
    T_MEM[0].print_contents();
    T_MEM[1].print_contents();
    T_MEM[0].victim.print_contents();
    cout << "\n\n";
#endif
  }

#if DEBUG
  cout << "Simulation done" << endl;
#endif
#if DEFBUG
  cout << "Printing L1" << endl;
#endif
  T_MEM[0].print_contents();
#if DEFBUG
  cout << "Printing L2" << endl;
#endif
  T_MEM[1].print_contents();
#if DEFBUG
  cout << "Printing victim" << endl;
#endif
  T_MEM[0].victim.print_contents();
}
