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

void simulate(total_cache &L1, total_cache &L2, total_cache &VC,
              AccessPattern &trace) {
  //
}
