#pragma once

#include <iostream>

#define LOG(msg) std::cout << "failed: " << (msg) << "\n";
#define ASSERT(condition) if(!(condition)) { LOG(#condition); return -1; }
