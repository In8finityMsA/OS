#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "employee.h"

int wmain(int argc, LPWSTR* argv) {
    std::wcout << "This is Creator utility.\n";
    if (argc != 3) {
        std::wcerr << "Not enough arguments. Usage: [output file] [records count]" << std::endl;
        system("pause");
        return -1;
    }

    LPCWSTR output_filename = argv[1];
    int records_count = wcstol(argv[2], NULL, 10);
    if (records_count <= 0) {
        std::wcerr << "Records count must be greater than zero" << std::endl;
        return -1;
    }

    std::ofstream fout;
    fout.open(output_filename, std::ofstream::binary);
    if (!fout)
    {
        std::wcerr << "Cannot open file: " << output_filename << std::endl;
        system("pause");
        return -1;
    }


    while (records_count > 0) {
        Employee input_employee;
        std::wcout << "Enter employee id: ";
        std::wcin >> input_employee.num;
        std::wcout << "Enter employee name (max 10 symbols): ";
        std::wstring name_str;
        std::wcin >> name_str;
        if (name_str.length() > 10) {
            std::wcout << "Notice: name is longer than 10 symbols, it will be cut." << std::endl;
        }
        wcsncpy(input_employee.name, name_str.c_str(), 10);
        std::wcout << "Enter employee hours: ";
        std::wcin >> input_employee.hours;
        std::wcout << "------------------------------\n";

        fout.write((const char*)&input_employee, sizeof(Employee));
        --records_count;
    }
    return 0;
}
