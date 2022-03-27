#include <iostream>
#include <Windows.h>

struct Array {
	int* data;
	size_t size;
};

struct Params {
	int num;
	Array* arr;

	HANDLE hStartEvent; 
	HANDLE hSuspendEvent;
	HANDLE hContinueEvent;
	HANDLE hEndEvent; 
	CRITICAL_SECTION* hCritSec;
};

DWORD WINAPI marker(LPVOID lpParam) {
	Params* p = (Params*)lpParam;
	Array* arr = p->arr;

	WaitForSingleObject(p->hStartEvent, INFINITE);

	int marked = 0;
	srand(p->num);
	while (true) {
		int rand_num = rand();
		int index = rand_num % arr->size;
		EnterCriticalSection(p->hCritSec);
		if (arr->data[index] == 0) {
			Sleep(5);
			arr->data[index] = p->num;
			marked++;
			LeaveCriticalSection(p->hCritSec);
			Sleep(5);
		} else {
			std::cout << "Thread #" << p->num << ", marked: " << marked << ", stuck on: " << index << std::endl;
			LeaveCriticalSection(p->hCritSec);
			SetEvent(p->hSuspendEvent);
			HANDLE next_event[] = {p->hContinueEvent, p->hEndEvent};

			if (WaitForMultipleObjects(2, next_event, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) { // End event signal
				for (int i = 0; i < arr->size; i++) {
					if (arr->data[i] == p->num) {
						arr->data[i] = 0;
					}
				}
				return 0;
			}
		}
	}

	
	return 0;
}

int main() {
	int n = 0;
	std::cout << "Enter array size: ";
	std::cin >> n;
	Array arr = { new int[n]{}, n };

	int num_marker = 0;
	std::cout << "Enter number of marker threads: ";
	std::cin >> num_marker;
	
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	HANDLE hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hStartEvent == NULL) {
		std::cerr << "Failed to create shared events!\n";
		return 1;
	}

	HANDLE* hEndEvents = new HANDLE[num_marker];
	HANDLE* hSuspendEvents = new HANDLE[num_marker];
	HANDLE* hContinueEvents = new HANDLE[num_marker];
	for (int i = 0; i < num_marker; i++) {
		hSuspendEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hEndEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hContinueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hSuspendEvents[i] == NULL || hEndEvents[i] == NULL || hContinueEvents[i] == NULL) {
			std::cerr << "Failed to create private events!\n";
			return 2;
		}
	}

	HANDLE* hMarkers = new HANDLE[num_marker];
	Params* params = new Params[num_marker];
	for (int i = 0; i < num_marker; i++) {
		params[i] = { i + 1, &arr, hStartEvent, hSuspendEvents[i], hContinueEvents[i], hEndEvents[i], &cs };
		CreateThread(NULL, 0, marker, &params[i], NULL, NULL);
		if (hMarkers[i] == NULL) {
			std::cerr << "Failed to create a thread!\n";
			return 3;
		}
	}

	SetEvent(hStartEvent);
	int left_threads = num_marker;
	bool* isStopped = new bool[num_marker]{};
	HANDLE* hSuspendUpdated = new HANDLE[num_marker]{};
	while (left_threads) {
		for (int cur = 0, i = 0; i < num_marker; i++) {
			if (!isStopped[i]) {
				hSuspendUpdated[cur++] = hSuspendEvents[i];
			}
		}

		WaitForMultipleObjects(left_threads, hSuspendUpdated, TRUE, INFINITE);
		std::cout << "Array after round " << (num_marker - left_threads + 1) << ":" << std::endl;
		for (int i = 0; i < n; i++) {
			std::cout << arr.data[i] << ' ';
		}
		std::cout << std::endl;

		int thread_num = 0;
		while (true) {
			std::cout << "Enter number of thread to end [" << 1 << ", " << num_marker << "]" << std::endl;
			
			std::cin >> thread_num;
			thread_num--;
			if (thread_num < 0 || thread_num >= num_marker) {
				std::cout << "Thread number is out of bounds" << std::endl;
				continue;
			}
			if (isStopped[thread_num]) {
				std::cout << "Thread was already stopped" << std::endl;
				continue;
			}
			break;
		}

		isStopped[thread_num] = true;
		SetEvent(hEndEvents[thread_num]);
		WaitForSingleObject(hMarkers[thread_num], INFINITE);
		CloseHandle(hSuspendEvents[thread_num]);
		--left_threads;

		std::cout << "Array after thread #" << thread_num + 1 << " elimination." << std::endl;
		for (int i = 0; i < n; i++) {
			std::cout << arr.data[i] << ' ';
		}
		std::cout << '\n' << std::endl;

		for (int i = 0; i < num_marker; i++) {
			SetEvent(hContinueEvents[i]);
		}
	}



	for (int i = 0; i < num_marker; i++) {
		CloseHandle(hMarkers[i]);
		CloseHandle(hSuspendEvents[i]);
		CloseHandle(hContinueEvents[i]);
		CloseHandle(hEndEvents[i]);
	}
	DeleteCriticalSection(&cs);
	CloseHandle(hStartEvent);
	delete[] isStopped;
	delete[] hSuspendUpdated;
	delete[] hEndEvents;
	delete[] hSuspendEvents;
	delete[] hContinueEvents;
	delete[] params;
	delete[] hMarkers;
	delete[] arr.data;
	return 0;
}