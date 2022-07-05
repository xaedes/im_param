#pragma once

#include <iostream>

#define LOG(msg) std::cout << "failed: " << (msg) << "\n";
#define LOG2(msg, value) std::cout << "failed: " << (msg) << "\n" << (#value) << " == " << (value) << "\n";
#define LOG3(msg, value, value2) std::cout << "failed: " << (msg) << "\n" << (#value) << " == " << (value) << "\n" (#value2) << " == " << (value2) << "\n";
#define ASSERT(condition) if(!(condition)) { LOG(#condition); return -1; }
#define ASSERT2(condition, value) if(!(condition)) { LOG2(#condition, value); return -1; }
#define ASSERT3(condition, value, value2) if(!(condition)) { LOG3(#condition, value, value2); return -1; }
