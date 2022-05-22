#pragma once
#include "pch.h"
#include <iostream>

#define CHECK_ERROR(error_cond, message) \
if (error_cond) { \
    std::wcerr << message << std::endl; \
    system("pause"); \
    return -1; \
} \