#pragma once

#include <stdexcept>
#include <string> 

#define RPR_CHECK(x)                             \
    {                                            \
        if ((x) != RPR_SUCCESS) {                \
            ThrowRprError(x, __FILE__, __LINE__); \
        }                                        \
    }

inline void ThrowRprError(int errorCode, const char* fileName, int line)
{
    throw std::runtime_error("RPR Error code = " + std::to_string(errorCode) 
        + "\nfile = " + std::string(fileName) 
        + "\nline = " + std::to_string(line));
}