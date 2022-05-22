#include "pch.h"
#include "employee.h"
#include <sstream>
#include <iostream>

std::wstring EmployeeToString(Employee employee) {
	std::wstring str = L"";
	std::wstringstream stream;
	stream << L"ID: " << employee.num << '\n';
	stream << L"Name: " << employee.name << '\n';
	stream << L"Hours: " << employee.hours;
	return stream.str();
}

Employee ReadEmployeeFromConsole(int id) {
	Employee input_employee{};
	if (id == 0) {
		std::wcout << "Enter employee id: ";
		std::wcin >> input_employee.num;
	} else {
		input_employee.num = id;
	}

	std::wcout << "Enter employee name (max " << name_length << " symbols): ";
	std::wstring name_str;
	std::wcin.ignore();
	getline(std::wcin, name_str);
	if (name_str.length() > name_length) {
		std::wcout << "Notice: name is longer than " << name_length << " symbols, it will be cut." << std::endl;
	}
	wcsncpy_s(input_employee.name, name_str.c_str(), name_length);

	std::wcout << "Enter employee hours: ";
	std::wcin >> input_employee.hours; // TODO: check negative
	return input_employee;
}