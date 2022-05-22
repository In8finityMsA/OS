#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <iomanip>
#include "employee.h"

#define NO_ERROR 0
#define MAX_BYTES_PER_LINE 8

std::wstring creator_app = L"Creator.exe";
std::wstring reporter_app = L"Reporter.exe";

INT32 ReadBinaryFile(const std::wstring& filename);
INT32 ReadReport(const std::wstring& filename);
INT32 CallUtility(LPCTSTR utilityName, const std::wstring& command_line);

INT32 main() {
    INT32 result;
    std::wcout << "This is a programm for employee managment.\n";

    std::wstring filename_data;
    std::wcout << "Please enter a name for a data file: ";
    getline(std::wcin, filename_data);

    std::wstring records;
    std::wcout << "Enter number of records to add: ";
    getline(std::wcin, records);

    std::wstring command_line = creator_app + L" " + filename_data + L" " + records;
    if (NO_ERROR != (result = CallUtility(creator_app.c_str(), command_line))) {
        std::wcerr << "Error in Creator utility: " << result << "\n";
        return result;
    }

    if (NO_ERROR != (result = ReadBinaryFile(filename_data))) {
        std::wcerr << "Error reading data file: " << result << "\n";
        return result;
    }

    std::wstring filename_report;
    std::wcout << "Please enter a name for a report file: ";
    getline(std::wcin, filename_report);

    std::wstring wage;
    std::wcout << "Enter a hourly wage: ";
    getline(std::wcin, wage);

    command_line = reporter_app + L" " + filename_data + L" " + filename_report + L" " + wage;
    if (NO_ERROR != (result = CallUtility(reporter_app.c_str(), command_line))) {
        std::wcerr << "Error in Reporter utility: " << result << "\n";
        return result;
    }

    if (NO_ERROR != (result = ReadReport(filename_report))) {
        std::wcerr << "Error reading report file: " << result << "\n";
        return result;
    }

    std::wcout << "Program ended execution successfully" << std::endl;
}

INT32 ReadBinaryFile(const std::wstring& filename) {
    std::ifstream fin;
    fin.open(filename, std::ifstream::binary);
    if (!fin) {
        std::wcerr << "Cannot open file: " << filename << std::endl;
        return -1;
    }
    std::wcout << "Data file contents:\n";

    INT32 bytes_in_line = 0;
    std::wcout.setf(std::ios::hex, std::ios::basefield);
    std::wcout.setf(std::ios::uppercase);
    std::wcout.setf(std::ios::right);
    while (!fin.eof()) {
        int byte = 0;
        fin.read((char*)&byte, sizeof(char));

        if (fin.eof()) {
            break;
        }
        else if (!fin.good()) {
            std::wcerr << "An error occured while reading a file." << std::endl;
            return -1;
        }

        if (bytes_in_line++ >= MAX_BYTES_PER_LINE) {
            bytes_in_line = 1;
            std::wcout << "\n";
        }
        std::wcout << std::setfill(L'0') << std::setw(2) << byte << " ";
    }
    std::wcout.unsetf(std::ios::hex);
    std::wcout.unsetf(std::ios::uppercase);
    std::wcout << "\n" << std::endl;
    return 0;
}

INT32 ReadReport(const std::wstring& filename) {
    std::wifstream fin;
    fin.open(filename, std::wifstream::in);
    if (!fin) {
        std::wcerr << "Cannot open file: " << filename << std::endl;
        return -1;
    }
    std::wcout << "Report file contents:\n";

    std::wstring line;
    while (getline(fin, line)) {
        if (!fin.good()) {
            std::wcerr << "An error occured while reading a file." << std::endl;
            return -1;
        }

        std::wcout << line << "\n";
    }
    std::wcout << std::endl;
    return 0;
}

INT32 CallUtility(LPCTSTR utility_name, const std::wstring& cmd_line_string) {
    LPWSTR cmd_line = new WCHAR[cmd_line_string.length() + 1];
    wcscpy_s(cmd_line, cmd_line_string.length() + 1, cmd_line_string.c_str());

    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION proc_info = { 0 };
    if (!CreateProcessW(utility_name, cmd_line, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &si, &proc_info)) {
        std::wcerr << "Create process failed" << std::endl;
        delete[] cmd_line;
        return -20;
    }
    WaitForSingleObject(proc_info.hProcess, INFINITE);
    delete[] cmd_line;

    DWORD exit_code;
    if (!GetExitCodeProcess(proc_info.hProcess, &exit_code)) {
        std::wcerr << "GetExitCodeProcess() failure: " << GetLastError() << "\n";
        return -21;
    } else if (exit_code != 0) {
        std::wcout << utility_name << " exited with error: " << exit_code << "\n";
        return exit_code;
    }
    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
    return 0;
}