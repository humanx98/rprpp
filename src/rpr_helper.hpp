#pragma once

#include <RadeonProRender.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define RPR_CHECK(x)                             \
    {                                            \
        if ((x) != RPR_SUCCESS) {                \
            ErrorManager(x, __FILE__, __LINE__); \
        }                                        \
    }

void ErrorManager(int errorCode, const char* fileName, int line);
void CheckNoLeak(rpr_context context);
rpr_shape ImportOBJ(const std::string& file, rpr_context ctx);