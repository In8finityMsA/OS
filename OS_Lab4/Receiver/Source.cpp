#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include "Constants.h"

char* ReadMessage(std::ifstream& input, char* buffer, size_t size_msg, size_t max_msg, HANDLE hSemEmptyCells, HANDLE hSemUnreadMsg, HANDLE hMutex) {
	WaitForSingleObject(hSemUnreadMsg, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

	auto cur_pos = input.tellg();
	if (cur_pos >= HEADER_SIZE + size_msg * max_msg - 1) {
		input.seekg(HEADER_SIZE);
	}
	input.read(buffer, size_msg);

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hSemEmptyCells, 1, NULL);
	return 0;
}

int main() {
	std::wstring filename;
	std::wcout << L"Enter binary file name: ";
	std::wcin >> filename;

	size_t max_msg;
	std::wcout << L"Enter maximum amount of messages: ";
	std::wcin >> max_msg;

	size_t size_msg;
	std::wcout << L"Enter size of a message: ";
	std::wcin >> size_msg;

	std::ofstream fout;
	fout.rdbuf()->pubsetbuf(0, 0);
	fout.open(filename, std::ios::binary | std::ios::out);
	fout.seekp(HEADER_SIZE + (max_msg * size_msg) - 1);
	fout.write("", 1);

	fout.seekp(0);
	size_t next_empty = HEADER_SIZE;
	fout.write((const char*)&next_empty, sizeof(next_empty));
	fout.close();

	size_t senders_count;
	std::cout << "Enter amount of senders: ";
	std::cin >> senders_count;

	std::wstring cmd_line = SENDER_EXEC;
	cmd_line.append(L" ").append(filename).append(L" ").append(std::to_wstring(max_msg)).append(L" ").append(std::to_wstring(size_msg)).append(L" ");


	auto* hEvents = new HANDLE[senders_count]{};
	auto* startup = new STARTUPINFO[senders_count]{};
	auto* proc_info = new PROCESS_INFORMATION[senders_count]{};
	auto* private_cmd_lines = new std::wstring[senders_count]{};

	HANDLE hMutex = CreateMutexW(NULL, FALSE, MUTEX_NAME);
	if (hMutex == NULL)
		return GetLastError();

	HANDLE hSemEmptyCells = CreateSemaphoreW(NULL, max_msg, max_msg, SEMEMPTY_NAME);
	HANDLE hSemUnreadMsg = CreateSemaphoreW(NULL, 0, max_msg, SEMFULL_NAME);
	if (hSemEmptyCells == NULL || hSemUnreadMsg == NULL)
		return GetLastError();

	for (size_t i = 0; i < senders_count; i++) {
		private_cmd_lines[i] = cmd_line;
		std::wstring eventName = EVENT_NAME;
		eventName.append(std::to_wstring(i));

		hEvents[i] = CreateEventW(NULL, FALSE, FALSE, eventName.c_str());
		if (hEvents[i] == NULL)
			return GetLastError();
		private_cmd_lines[i].append(eventName);
	}

	for (size_t i = 0; i < senders_count; i++) {
		startup[i].cb = sizeof(startup[i]);
		if (!CreateProcessW(SENDER_EXEC, const_cast<wchar_t*>(private_cmd_lines[i].c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startup[i], &proc_info[i])) {
			std::wcerr << "CreateProcess failed.";
			return -1;
		}
	}

	WaitForMultipleObjects(senders_count, hEvents, TRUE, INFINITE);
	std::wcout << "Events are set, let's start!" << std::endl;

	int answer = -1;
	std::ifstream fin;
	fin.rdbuf()->pubsetbuf(0, 0);
	fin.open(filename, std::ios::binary | std::ios::in);
	fin.seekg(HEADER_SIZE);
	char* msg_buffer = new char[size_msg + 1];
	msg_buffer[size_msg] = '\0';

	do {
		answer = -1;
		std::wcout << L"Enter command (1 - Read message, 0 - Exit program): ";
		std::cin >> answer;
		if (answer == 1) {
			auto result = ReadMessage(fin, msg_buffer, size_msg, max_msg, hSemEmptyCells, hSemUnreadMsg, hMutex);
			std::cout << "Read message: " << msg_buffer << std::endl;
		} else if (answer != 0) {
			std::wcout << L"Please enter 0 or 1\n";
			std::wcin.clear();
		}
	} while (answer != 0);
	std::wcout << L"Program was terminated by user.";

	
	CloseHandle(hMutex);
	CloseHandle(hSemEmptyCells);
	CloseHandle(hSemUnreadMsg);
	//CloseHandle(hStartEvent);
	fin.close();
	delete[] msg_buffer;
	delete[] private_cmd_lines;
	delete[] startup;
	delete[] hEvents;
	delete[] proc_info;
	return 0;
}