#include <cstdlib> // For atoi function
#include <cstring> // For strcmp function
#include <iostream>

#include "../include/cache.h"
#include "../include/simulator.h"

#define DEBUG 1

// Function to display help message
void printHelp() {
  std::cout << "Usage: cache_sim <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> "
               "<VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>\n";
  std::cout << "Arguments:\n";
  std::cout << "  L1_SIZE         : L1 cache size in Bytes.\n";
  std::cout << "  L1_ASSOC        : L1 set-associativity.\n";
  std::cout << "  L1_BLOCKSIZE    : L1 block size in Bytes.\n";
  std::cout << "  VC_NUM_BLOCKS   : Number of blocks in the Victim Cache (0 if "
               "no VC).\n";
  std::cout << "  L2_SIZE         : L2 cache size in bytes. (0 if no L2)\n";
  std::cout << "  L2_ASSOC        : L2 set-associativity.\n";
  std::cout << "  trace_file      : Character string specifying the full name "
               "of trace file.\n";
  std::cout << "Example: cache_sim 16384 4 64 4 65536 8 trace.txt\n";
}

void make_cache(uint size, uint blocksize, uint assoc) {}

// Main function to handle command-line arguments
int main(int argc, char *argv[]) {
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
  // Check if the number of arguments is correct
  if (argc != 8) {
    std::cerr << "Error: Incorrect number of arguments.\n";
    printHelp();
    return 1;
  }

  // Convert input arguments to appropriate data types
  uint L1_SIZE = std::atoi(argv[1]);
  uint L1_ASSOC = std::atoi(argv[2]);
  uint L1_BLOCKSIZE = std::atoi(argv[3]);
  uint VC_NUM_BLOCKS = std::atoi(argv[4]);
  uint L2_SIZE = std::atoi(argv[5]);
  uint L2_ASSOC = std::atoi(argv[6]);
  const char *trace_file = argv[7];

  // Validate the inputs
  if (L1_SIZE <= 0 || L1_ASSOC <= 0 || L1_BLOCKSIZE <= 0 || VC_NUM_BLOCKS < 0 ||
      L2_SIZE < 0 || L2_ASSOC <= 0) {
    std::cerr << "Error: Invalid input values.\n";
    printHelp();
    return 1;
  }

  if (L2_SIZE == 0 && L2_ASSOC != 0) {
    std::cerr << "Error: L2_ASSOC must be 0 if L2_SIZE is 0.\n";
    printHelp();
    return 1;
  }

#if DEBUG
  // Output the input values (for verification)
  std::cout << "L1 Cache Size (Bytes): " << L1_SIZE << "\n";
  std::cout << "L1 Cache Associativity: " << L1_ASSOC << "\n";
  std::cout << "L1 Cache Block Size (Bytes): " << L1_BLOCKSIZE << "\n";
  std::cout << "Victim Cache Number of Blocks: " << VC_NUM_BLOCKS << "\n";
  std::cout << "L2 Cache Size (Bytes): " << L2_SIZE << "\n";
  std::cout << "L2 Cache Associativity: " << L2_ASSOC << "\n";
  std::cout << "Trace File: " << trace_file << "\n";
#endif

  // Continue with further processing as needed...

  std::vector<AccessPattern> Accesses;
  parser(trace_file, Accesses);

  total_cache L1_C(L1_ASSOC, L1_BLOCKSIZE, L1_SIZE);
  total_cache L2_C(L2_ASSOC, L1_BLOCKSIZE, L2_SIZE); // to correct later
  total_cache V_C; // check if we can instead make it a base cache

  return 0;
}
