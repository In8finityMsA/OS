#pragma once
#include <Windows.h>
#include <string>

#define CHECK_ERROR(error_cond, message) \
if (error_cond) { \
    std::wcerr << message << std::endl; \
    system("pause"); \
    return -1; \
} \

DWORD WINAPI ServerThread(LPVOID args);

struct ServerThreadParam {
    HANDLE hPipe;
    std::wstring filename;
    int thread_index;
    int* num_records;
    int* cur_readers;
    HANDLE* mutex_lock;
    HANDLE* write_available;
};