#pragma once

#include "cache.h"
#include <fstream>
#include <iomanip> // For std::hex
#include <iostream>
#include <sstream>
#include <string>
#include <utility> // For std::pair
#include <vector>

using AccessPattern = std::pair<char, unsigned int>;
using uint = unsigned int;

void parser(const std::string &filename,
            std::vector<AccessPattern> accessPatterns);

void simulate(total_cache &L1, total_cache &L2, total_cache &VC,
              AccessPattern &trace);
