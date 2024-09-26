#include <cstdlib> // For atoi function
#include <cstring> // For strcmp function
#include <iostream>
#include <string>
#include <vector>

#include "../include/cache.h"
#include "../include/cacti_parser.cpp"
#include "../include/simulator.h"

#define DEBUG 0
#define LEVELS 2

using namespace std;

// Function to display help message
void printHelp() {
  cout << "Usage: cache_sim <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> "
          "<VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>\n";
  cout << "Arguments:\n";
  cout << "  L1_SIZE         : L1 cache size in Bytes.\n";
  cout << "  L1_ASSOC        : L1 set-associativity.\n";
  cout << "  L1_BLOCKSIZE    : L1 block size in Bytes.\n";
  cout << "  VC_NUM_BLOCKS   : Number of blocks in the Victim Cache (0 if "
          "no VC).\n";
  cout << "  L2_SIZE         : L2 cache size in bytes. (0 if no L2)\n";
  cout << "  L2_ASSOC        : L2 set-associativity.\n";
  cout << "  trace_file      : Character string specifying the full name "
          "of trace file.\n";
  cout << "Example: cache_sim 16384 4 64 4 65536 8 trace.txt\n";
}

void make_cache(uint size, uint blocksize, uint assoc) {}

// Main function to handle command-line arguments
int main(int argc, char *argv[]) {
  // Check if the number of arguments is correct
  if (argc != 8) {
    cerr << "Error: Incorrect number of arguments.\n";
    printHelp();
    return 1;
  }

  // Convert input arguments to appropriate data types
  uint L1_SIZE = atoi(argv[1]);
  uint L1_ASSOC = atoi(argv[2]);
  uint L1_BLOCKSIZE = atoi(argv[3]);
  uint VC_NUM_BLOCKS = atoi(argv[4]);
  uint L2_SIZE = atoi(argv[5]);
  uint L2_ASSOC = atoi(argv[6]);
  const char *trace_file = argv[7];

  // Validate the inputs
  if (L1_SIZE <= 0 || L1_ASSOC <= 0 || L1_BLOCKSIZE <= 0 || VC_NUM_BLOCKS < 0 ||
      L2_SIZE < 0 || L2_ASSOC < 0) {
    cerr << "Error: Invalid input values.\n";
    printHelp();
    return 1;
  }

  if (L2_SIZE == 0 && L2_ASSOC != 0) {
    cerr << "Error: L2_ASSOC must be 0 if L2_SIZE is 0.\n";
    printHelp();
    return 1;
  }

  //
  cout << "===== Simulator configuration =====" << endl
       << "L1_SIZE:		" << L1_SIZE << endl
       << "L1_ASSOC:		" << L1_ASSOC << endl
       << "L1_BLOCKSIZE:		" << L1_BLOCKSIZE << endl
       << "VC_NUM_BLOCKS:	" << VC_NUM_BLOCKS << endl
       << "L2_SIZE:		" << L2_SIZE << endl
       << "L2_ASSOC:		" << L2_ASSOC << endl
       << "trace_file:		" << trace_file << endl
       << endl;
  //

#if DEBUG
  // Output the input values (for verification)
  cout << "L1 Cache Size (Bytes): " << L1_SIZE << "\n";
  cout << "L1 Cache Associativity: " << L1_ASSOC << "\n";
  cout << "L1 Cache Block Size (Bytes): " << L1_BLOCKSIZE << "\n";
  cout << "Victim Cache Number of Blocks: " << VC_NUM_BLOCKS << "\n";
  cout << "L2 Cache Size (Bytes): " << L2_SIZE << "\n";
  cout << "L2 Cache Associativity: " << L2_ASSOC << "\n";
  cout << "Trace File: " << trace_file << "\n";
#endif

  // Continue with further processing as needed...

  vector<AccessPattern> Accesses;
  parser(trace_file, Accesses);

  // can make multiple level support later by putting an array

  vector<total_cache> T_MEM;
#if DEBUG
  cout << "Making L1" << endl;
#endif
  T_MEM.emplace_back(L1_ASSOC, L1_BLOCKSIZE, L1_SIZE, VC_NUM_BLOCKS);

#if DEBUG
  cout << "Making L2" << endl;
#endif
  T_MEM.emplace_back(L2_ASSOC, L1_BLOCKSIZE, L2_SIZE, 0);

  // simulate(T_MEM, LEVELS, Accesses);
  temp_simulate(T_MEM, LEVELS, Accesses);

  return 0;
}
