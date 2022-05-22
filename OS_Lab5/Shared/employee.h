#pragma once
#include "pch.h"
#include <string>

constexpr size_t name_length = 9;

struct Employee {
	int num;
	wchar_t name[name_length + 1];
	double hours;
};

std::wstring EmployeeToString(Employee employee);

Employee ReadEmployeeFromConsole(int id = 0);