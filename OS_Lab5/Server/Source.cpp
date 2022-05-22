#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include "employee.h"
#include "server.h"


INT32 ReadBinaryFile(const std::wstring& filename);
INT32 WriteBinaryFile(const std::wstring& filename, int* records_count);

std::wstring client_process_name = L"Client.exe";

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    std::wstring output_filename;
    std::wcout << "Enter file name: ";
    std::wcin >> output_filename;

    int records_count = 0;
    CHECK_ERROR(WriteBinaryFile(output_filename, &records_count) != 0, L"Error writing to file.");
    CHECK_ERROR(ReadBinaryFile(output_filename) != 0, L"Error reading from file.");

    int client_num = -1;
    std::wcout << "Enter number of client processes: " << std::endl;
    std::cin >> client_num;
    CHECK_ERROR(client_num <= 0, L"Clients number must be greater than zero.");

    CRITICAL_SECTION cs;
    auto* si = new STARTUPINFO[client_num]{ 0 };
    auto* pi = new PROCESS_INFORMATION[client_num]{ 0 };
    auto* thread_params = new ServerThreadParam[client_num];
    auto* hServer = new HANDLE[client_num];
    auto* serverID = new DWORD[client_num];
    InitializeCriticalSection(&cs);
    
    int current_readers = 0;
    auto* mutexes = new HANDLE[records_count];
    auto* events = new HANDLE[records_count];
    for (size_t i = 0; i < records_count; i++) {
        mutexes[i] = CreateMutexW(nullptr, FALSE, nullptr);
        if (mutexes[i] == nullptr || mutexes[i] == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed creating mutex" << i << ". Error " << GetLastError() << std::endl;
            return -1;
        }

        events[i] = CreateEventW(nullptr, FALSE, TRUE, nullptr);
        if (events[i] == nullptr || events[i] == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed creating event" << i << ". Error " << GetLastError() << std::endl;
            return -1;
        }
    }

    for (size_t i = 0; i < client_num; i++) {
        std::wstring pipe_name = LR"(\\.\pipe\OS_lab5\client)" + std::to_wstring(i);
        HANDLE hPipe = CreateNamedPipeW(
            pipe_name.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE,
            1,
            0,
            0,
            0,
            nullptr);

        if (hPipe == nullptr || hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed creating pipe " << i << ". Error " << GetLastError() << std::endl;
            return -1;
        }

        thread_params[i].hPipe = hPipe;
        thread_params[i].filename = output_filename;
        thread_params[i].thread_index = i;
        thread_params[i].num_records = &records_count;
        thread_params[i].cur_readers = &current_readers;
        thread_params[i].mutex_lock = mutexes;
        thread_params[i].write_available = events;
        hServer[i] = CreateThread(
            nullptr,
            0,
            ServerThread,
            (void*)&thread_params[i],
            0,
            serverID + i);
        if (hServer[i] == nullptr || hServer[i] == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed creating server thread " << i << ". Error " << GetLastError() << std::endl;
            return -1;
        }

        si[i].cb = sizeof(si[i]);
        std::wstring cmdLine = client_process_name + L" " + output_filename + L" " + pipe_name;
        if (!CreateProcessW(
            client_process_name.c_str(),
            const_cast<wchar_t*>(cmdLine.c_str()),
            nullptr,
            nullptr,
            false,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si[i],
            &pi[i])
            ) {
            std::cerr << "Failed creating process " << i << ". Error " << GetLastError() << std::endl;
            return -1;
        }

    }
    auto* processes = new HANDLE[client_num];
    for (size_t i = 0; i < client_num; i++) {
        processes[i] = pi[i].hProcess;
    }
    WaitForMultipleObjects(client_num, processes, TRUE, INFINITE); // Wait for all clients to stop

    std::cout << "All clients have ended execution. Server exits.";
    for (size_t i = 0; i < client_num; i++) {
        CloseHandle(thread_params[i].hPipe);
    }
    return 0;
}

INT32 WriteBinaryFile(const std::wstring& filename, int* records_count) {
    std::fstream binary_file;
    binary_file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!binary_file) {
        std::wcerr << "Cannot open file: " << filename << std::endl;
        system("pause");
        return -1;
    }

    *records_count = -1;
    std::wcout << "Enter number of records: ";
    std::wcin >> *records_count;
    if (*records_count <= 0) {
        std::wcerr << "Records count must be greater than zero." << std::endl;
        system("pause");
        return -1;
    }

    for (int id = 1; id <= *records_count; id++) {
        Employee input_employee = ReadEmployeeFromConsole(id);
        std::wcout << "------------------------------\n";

        binary_file.write((const char*)&input_employee, sizeof(Employee));
        binary_file.flush();
    }
    return 0;
}

INT32 ReadBinaryFile(const std::wstring& filename) {
    std::ifstream fin;
    fin.open(filename, std::ifstream::binary);
    if (!fin) {
        std::wcerr << "Failed opening file: " << filename << std::endl;
        return -1;
    }
    std::wcout << "Data file contents:\n";

    while (!fin.eof()) {
        Employee buffer_employee{ 0 };
        fin.read((char*)&buffer_employee, sizeof(Employee));

        if (fin.eof()) {
            break;
        } else if (!fin.good()) {
            std::wcerr << "An error occured while reading a file." << std::endl;
            return -1;
        }

        std::wcout << EmployeeToString(buffer_employee) << std::endl;
    }
    std::wcout << std::endl;
    return 0;
}