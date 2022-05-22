#include <iostream>
#include <fstream>
#include <Windows.h>
#include <Constants.h>
#include <string>

int WriteMessage(std::fstream& output, const char* msg, size_t size_msg, size_t max_msg, HANDLE hSemEmptyCells, HANDLE hSemUnreadMsg, HANDLE hMutex) {
	WaitForSingleObject(hSemEmptyCells, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

	output.seekg(0);
	uint32_t write_pos = 0;
	output.read((char *)&write_pos, sizeof(write_pos));

	output.seekp(write_pos);
	output.write(msg, size_msg);

	output.seekp(0);
	write_pos += size_msg;
	if (write_pos >= HEADER_SIZE + size_msg * max_msg - 1) {
		output.seekg(HEADER_SIZE);
	}
	output.write((const char*)&write_pos, sizeof(write_pos));

	output.flush();

	ReleaseMutex(hMutex);
	ReleaseSemaphore(hSemUnreadMsg, 1, NULL);
	return 0;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

	if (argc != 5) {
		std::wcout << "Wrong number of arguments, expected 4" << std::endl;
		std::wcout << "Press any key to exit." << std::endl;
		std::wcin.get();
	}

	size_t max_msg = 0;
	size_t size_msg = 0;
	max_msg = std::stoi(argv[2]);
	size_msg = std::stoi(argv[3]); // TODO: catch exc

	HANDLE hStartEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, argv[4]);
	if (hStartEvent == NULL) {
		std::wcout << "Open event failed." << std::endl;
		std::wcout << "Press any key to exit." << std::endl;
		std::wcin.get();

		return GetLastError();
	}

	HANDLE hSemEmptyCells = OpenSemaphoreW(SEMAPHORE_MODIFY_STATE, FALSE, SEMEMPTY_NAME);
	if (hSemEmptyCells == NULL) {
		std::wcout << "Open semaphore - empty failed." << std::endl;
		std::wcout << "Press any key to exit." << std::endl;
		std::wcin.get();

		return GetLastError();
	}

	HANDLE hSemUnreadMsg = OpenSemaphoreW(SEMAPHORE_MODIFY_STATE, FALSE, SEMFULL_NAME);
	if (hSemUnreadMsg == NULL) {
		std::wcout << "Open semaphore - full failed." << std::endl;
		std::wcout << "Press any key to exit." << std::endl;
		std::wcin.get();

		return GetLastError();
	}

	HANDLE hMutex = OpenMutexW(MUTEX_MODIFY_STATE, FALSE, MUTEX_NAME);
	if (hSemUnreadMsg == NULL) {
		std::wcout << "Open mutex failed." << std::endl;
		std::wcout << "Press any key to exit." << std::endl;
		std::wcin.get();

		return GetLastError();
	}

	SetEvent(hStartEvent);
	std::wcout << "Event set." << std::endl;
	
	int answer = -1;
	std::fstream fstream;
	fstream.rdbuf()->pubsetbuf(0, 0);
	//fstream.buf()->pubsetbuf(0, 0);
	fstream.open(argv[1], std::ios::binary | std::ios::out | std::ios::in);
	char* msg_buffer = new char[size_msg] {};

	do {
		answer = -1;
		std::wcout << L"Enter command (1 - Write message, 0 - Exit program): ";
		std::wcin >> answer;
		if (answer == 1) {
			std::string msg;
			std::wcout << "Enter your message: ";
			std::cin.ignore();
			std::getline(std::cin, msg);
			memset(msg_buffer, 0, size_msg);
			strcpy_s(msg_buffer, size_msg, msg.c_str());
			
			auto result = WriteMessage(fstream, msg_buffer, size_msg, max_msg, hSemEmptyCells, hSemUnreadMsg, hMutex);
		} else if (answer != 0) {
			std::wcout << L"Please enter 0 or 1\n";
			std::wcin.clear();
		}
	} while (answer != 0);
	std::wcout << L"Program was terminated by user.";

	delete[] msg_buffer;
	CloseHandle(hMutex);
	CloseHandle(hSemEmptyCells);
	CloseHandle(hSemUnreadMsg);
	CloseHandle(hStartEvent);
	fstream.close();
	return 0;
}