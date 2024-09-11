#include <cstdlib>
#include <fstream>
#include <iomanip> // For std::hex
#include <iostream>
#include <sstream>
#include <string>
#include <utility> // For std::pair
#include <vector>

#include "../include/simulator.h"
// Type alias for better readability

void parser(const std::string &filename,
            std::vector<AccessPattern> accessPatterns) {
  // std::vector<AccessPattern>
  //     accessPatterns;           // Vector to store the parsed access patterns
  std::ifstream file(filename); // Open the file

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    exit(1);
    // return accessPatterns;
  }

  std::string line;
  while (std::getline(file, line)) { // Read the file line by line
    std::istringstream iss(line);
    char operation;         // 'r' or 'w'
    std::string addressHex; // Address in hexadecimal format

    // Parse the operation and address from the line
    if (iss >> operation >> addressHex) {
      // Convert the hexadecimal address to unsigned int
      unsigned int address;
      std::stringstream ss;
      ss << std::hex << addressHex;
      ss >> address;

      // Store the parsed result in the vector
      accessPatterns.emplace_back(operation, address);
    }
  }

  file.close(); // Close the file
  // return accessPatterns;
}

void simulate(vector<total_cache> &T_MEM, uint LEVELS,
              vector<AccessPattern> &trace, int has_Victim) {
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
  bool in_L1 = 0;
  bool in_VC = 0;
  bool in_L2;

  for (uint i = 0; i < trace.size(); i++) {
    char type = trace[i].first;
    uint addr = trace[i].second;
    // try in L1
    if (type == 'r') {
      L1_reads++;
    } else if (type == 'w') {
      L1_writes++;
    } else {
      cout << "WTF is that letter\n";
      exit(1);
    }
    in_L1 = T_MEM[0].access(addr, type);
    if (in_L1) {
      T_MEM[0].update_lru(addr);
    } else {
      if (type == 'r') {
        L1_read_misses++;
      } else if (type == 'w') {
        L1_write_misses++;
      }
      if (has_Victim) {
        VC_swap_req++;
        in_VC = T_MEM[0].check_in_victim(addr);
        if (in_VC) {
          No_of_Swaps++;
        }
      }
    }
    if (!in_L1 && !in_VC) {
      if (type == 'r') {
        L2_reads++;
      } else if (type == 'w') {
        L2_writes++;
      }
      in_L2 = T_MEM[1].access(addr, type);
      T_MEM[1].update_lru(addr);
      // since L2 has, move this to L1 and handle appropriately
      bool empty = 0;
      int dirty = 0;
      uint evict = T_MEM[0].put_it_inside(addr, empty, dirty);
      if (!empty) {
        evict = T_MEM[0].victim.insert(evict, dirty);
        if (dirty) {
          // block wass dirty write into L2
          T_MEM[1].put_it_inside(evict, empty, dirty);
        }
      }
      if (!in_L2) {
        T_MEM[1].put_it_inside(addr, empty, dirty);
      }
    }
  }
}
