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
              vector<AccessPattern> &trace) {
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
  for (uint i = 0; i < LEVELS; i++) {
  }
}
