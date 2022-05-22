#include <iostream>
#include <fstream>
#include <windows.h>
#include "employee.h"

INT32 wmain(int argc, LPWSTR* argv) {
    std::wcout << "This is Reporter utility.\n";
    if (argc != 4) {
        std::cerr << "Not enough arguments. Usage: [input file] [output file] [hourly wage]" << std::endl;
        system("pause");
        return -1;
    }

    LPCWSTR input_filename = argv[1];
    LPCWSTR output_filename = argv[2];
    INT32 hourly_wage = wcstol(argv[3], NULL, 10);
    if (hourly_wage <= 0) {
        std::wcerr << "Hourly wage must be greater than zero" << std::endl;
        return -1;
    }

    std::ifstream fin;
    fin.open(input_filename, std::ifstream::binary);
    if (!fin) {
        std::wcerr << "Cannot open file: " << input_filename << std::endl;
        system("pause");
        return -1;
    }

    std::wofstream reportFileOut;
    reportFileOut.open(output_filename, std::ofstream::out);
    if (!reportFileOut) {
        std::wcerr << "Cannot open file: " << output_filename << std::endl;
        system("pause");
        return -1;
    }

    reportFileOut << "Report generated from file " << input_filename << '\n';
    INT32 total_records = 0;
    while (!fin.eof()) {
        Employee input_employee;
        fin.read((char*)&input_employee, sizeof(Employee));

        if (fin.eof()) { 
            break;
        }
        else if (!fin.good()) {
            std::wcerr << "An error occured while reading a file." << std::endl;
            return -1;
        }
        reportFileOut << input_employee.num << ' '
            << input_employee.name << ' '
            << input_employee.hours << ' '
            << input_employee.hours * hourly_wage << std::endl;
        ++total_records;
    }
    reportFileOut << "End of report. Total records: " << total_records << std::endl;

    return 0;
}