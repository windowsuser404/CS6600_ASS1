#pragma once

#include "cache.h"
#include <fstream>
#include <iomanip> // For hex
#include <iostream>
#include <sstream>
#include <string>
#include <utility> // For pair
#include <vector>

using namespace std;

using AccessPattern = pair<char, unsigned int>;
using uint = unsigned int;

void parser(const string &filename, vector<AccessPattern> accessPatterns);

void simulate(vector<total_cache> &T_MEM, uint LEVELS,
              vector<AccessPattern> &trace);
