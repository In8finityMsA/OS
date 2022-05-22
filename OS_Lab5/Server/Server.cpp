#include <Windows.h>
#include <iostream>
#include <fstream>
#include "employee.h"
#include "server.h"
#include "request.h"
#include <optional>

Employee ReadEmployee(std::wstring filename, size_t index);

int HandleRead(ServerThreadParam param, Request request) {
    Employee employee{-1, L"", -1};
    int index = request.index;
    if (index <= 0 || index > *param.num_records) {
        std::cerr << "Incorrect index: " << index << std::endl;
    }

    WaitForSingleObject(param.mutex_lock[index], INFINITE);
    ++(*param.cur_readers);
    if (*param.cur_readers == 1) {
        WaitForSingleObject(param.write_available[index], INFINITE);
    }
    ReleaseMutex(param.mutex_lock[index]);

    // read
    employee = ReadEmployee(param.filename, index);
    if (employee.num != index) {
        std::wcerr << "Not found employee with id: " << index << ", instead found: " << employee.num << std::endl;
    }

    WaitForSingleObject(param.mutex_lock[index], INFINITE);
    --(*param.cur_readers);
    if (*param.cur_readers == 0) {
        SetEvent(param.write_available[index]);
    }
    ReleaseMutex(param.mutex_lock[index]);

    CHECK_ERROR(!WriteFile(param.hPipe, &employee, sizeof(employee), nullptr, nullptr), "Failed sending data.");
    std::wcout << "Send data to client" << std::endl;
    return 0;
}

Employee ReadEmployee(std::wstring filename, size_t index) {
    std::ifstream fin(filename, std::ios::in | std::ios::binary);

    Employee employee{};
    fin.seekg(sizeof(Employee) * (index-1), std::ios_base::beg);
    fin.read((char*)&employee, sizeof(Employee));

    return employee;
}

Employee WriteEmployee(std::wstring filename, size_t index, Employee employee) {
    std::fstream fout(filename, std::ios::in | std::ios::out | std::ios::ate | std::ios::binary);

    fout.seekp(sizeof(Employee) * (index - 1), std::ios_base::beg);
    fout.write((const char*)&employee, sizeof(Employee));
    fout.flush();

    return employee;
}

int HandleWrite(ServerThreadParam param, Request request) {
    Employee employee{ -1, L"", -1 };
    int index = request.index;
    if (index <= 0 || index > *param.num_records) {
        std::cerr << "Incorrect index: " << index << std::endl;
    }

    WaitForSingleObject(param.mutex_lock[index], INFINITE);
    WaitForSingleObject(param.write_available[index], INFINITE);

    //Read
    employee = ReadEmployee(param.filename, index);
    if (employee.num != index) {
        std::wcerr << "Not found employee with id: " << index << ", instead found: " << employee.num << std::endl;
    }
    CHECK_ERROR(!WriteFile(param.hPipe, &employee, sizeof(employee), nullptr, nullptr), "Failed sending data.");

    ReadFile(param.hPipe, &employee, sizeof(employee), nullptr, nullptr);
    WriteEmployee(param.filename, index, employee);

    SetEvent(param.write_available[index]);
    ReleaseMutex(param.mutex_lock[index]);

    return 0;
}

DWORD WINAPI ServerThread(LPVOID args) {
    std::wcerr << sizeof(Employee) << std::endl;
    ServerThreadParam param = *(ServerThreadParam*)args;
    bool is_connected = ConnectNamedPipe(param.hPipe, nullptr);
    if (!is_connected) {
        std::wcerr << "Failed connecting to named pipe " << param.thread_index << ". Error " << GetLastError() << std::endl;
        return -1;
    }

    Request request{};
    do {
        ReadFile(param.hPipe, &request, sizeof(request), nullptr, nullptr);

        switch (request.type) {
        case Request::READ:
            HandleRead(param, request);
            break;
        case Request::WRITE:
            HandleWrite(param, request);
            break;
        }
    } while (request.type != Request::CLOSE);

    DisconnectNamedPipe(param.hPipe);
    CloseHandle(param.hPipe);
    return 0;
}

