#pragma once

#include <iostream>

#define _LOG_ERROR(x) std::cerr << (x) << std::endl;

#define DEBUG

#ifdef DEBUG
#define LOG_ERROR(x) do { _LOG_ERROR(x) } while (0)
#else
#define LOG_ERROR(x) do {} while(0)
#endif
