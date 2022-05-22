#include <iostream>
#include <fstream>
#include <Windows.h>
#include "employee.h"
#include "request.h"
#include <conio.h>
#include <optional>

#define CHECK_ERROR(error_cond, message) \
if (error_cond) { \
    std::wcerr << message << std::endl; \
    system("pause"); \
    return -1; \
} \

int HandleRead(HANDLE hPipe) {
    int id = -1;
    Employee employee{};

    std::wcout << "Start read command.\n";
    std::wcout << "Enter employee id: ";
    std::wcin >> id;
    Request request{ Request::READ, id };

    CHECK_ERROR(!WriteFile(hPipe, &request, sizeof(request), nullptr, nullptr), "Failed sending data.");

    ReadFile(hPipe, &employee, sizeof(employee), nullptr, nullptr);
    if (employee.num == -1 && employee.hours == -1) {
        std::wcerr << "Unable to find record with id: " << id << std::endl;
    } else {
        std::wcout << "Received data:\n" << EmployeeToString(employee) << std::endl;
    }
    return 0;
}

int HandleWrite(HANDLE hPipe) {
    int id = -1;
    Employee employee{};

    std::wcout << "Start write command.\n";
    std::wcout << "Enter employee id: ";
    std::wcin >> id;
    Request request{ Request::WRITE, id };

    CHECK_ERROR(!WriteFile(hPipe, &request, sizeof(request), nullptr, nullptr), "Failed sending data.");

    ReadFile(hPipe, &employee, sizeof(employee), nullptr, nullptr);
    if (employee.num == -1 && employee.hours == -1) {
        std::wcerr << "Unable to find record with id " << id << std::endl;
    } else {
        std::wcout << "Received data:\n" << EmployeeToString(employee) << std::endl;

        employee = ReadEmployeeFromConsole(employee.num);
        if (!WriteFile(hPipe, &employee, sizeof(employee), nullptr, nullptr)) {
            std::wcerr << "Failed sending modified data." << std::endl;
        } else {
            std::wcout << "Successfully sent modified data" << std::endl;
        }
    }
    return 0;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    CHECK_ERROR(argc != 3, "Wrong number of arguments, expected 2.");

	std::wstring file_name = argv[1];
	std::wstring pipe_name = argv[2];
	std::wcout << "Hello from client " << file_name << " " << pipe_name << std::endl;
    std::wcerr << sizeof(Employee) << std::endl;

	HANDLE hPipe = CreateFileW(
		pipe_name.c_str(),
		GENERIC_ALL,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr);

    CHECK_ERROR(hPipe == INVALID_HANDLE_VALUE || hPipe == nullptr, "Failed opening pipe.");
	std::wcout << "Opened successfully\n";

	int command = -1;
    do {
        std::wcout << "Enter command (Read - 0, Write - 1, Exit - 2): " << std::endl;
        std::wcin >> command;
        switch (command) {
        case 0: {
            HandleRead(hPipe);
            break;
        }

        case 1: {
            HandleWrite(hPipe);
            break;
        }

        case 2: {
            Request request{ Request::CLOSE, 0 };
            CHECK_ERROR(!WriteFile(hPipe, &request, sizeof(request), nullptr, nullptr), "Failed to send data.");
            break;
        }
        default:
            std::wcout << "Unknown command. Try again.\n";
        }
        std::wcout << std::endl;
    } while (command != 2);


    CloseHandle(hPipe);
	system("pause");
	return 0;
}