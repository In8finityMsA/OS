#include <stdio.h>
#include <windows.h>

// Empty define to mark out variables
#define OUT

struct Array {
    int* data;
    int size;
};

struct MinMaxParam {
    Array* arr;
    OUT int min;
    OUT int max;
};

struct AvgParam {
    Array* arr;
    OUT double avg;
};

DWORD WINAPI MinMax(LPVOID lpParam) {
    MinMaxParam* param = (MinMaxParam*)lpParam;
    if (param == NULL || param->arr == NULL || param->arr->size <= 0) {
        return -1;
    }
    Array* arr = param->arr;

    int max = arr->data[0], min = arr->data[0];
    for (int i = 0; i < arr->size; i++) {
        if (arr->data[i] < min) {
            min = arr->data[i];
            Sleep(7);
        }
        else if (arr->data[i] > max) {
            max = arr->data[i];
            Sleep(7);
        }
    }
    param->min = min;
    param->max = max;
    return 0;
}

DWORD WINAPI Avg(LPVOID lpParam) {
    AvgParam* param = (AvgParam*)lpParam;
    if (param == NULL || param->arr == NULL || param->arr->size <= 0) {
        return -1;
    }
    Array* arr = param->arr;

    double avg = 0;
    for (int i = 0; i < arr->size; i++) {
        avg += arr->data[i];
        Sleep(12);
    }
    avg /= arr->size;
    param->avg = avg;
    return 0;
}

INT32 main() {
    INT32 n = 0;
    printf("Enter array length: ");
    scanf_s("%d", &n);

    printf("Enter array elements: ");
    Array arr{ 0 };
    arr.data = new INT32[n];
    arr.size = n;
    for (INT32 i = 0; i < arr.size; i++) {
        scanf_s("%d", &arr.data[i]);
    }

    DWORD min_max_id = 0;
    MinMaxParam min_max_param{0};
    min_max_param.arr = &arr;
    HANDLE h_min_max = CreateThread(NULL, 0, MinMax, &min_max_param, 0, &min_max_id);

    DWORD avg_id = 0;
    AvgParam avg_param{ 0 };
    avg_param.arr = &arr;
    HANDLE h_avg = CreateThread(NULL, 0, Avg, &avg_param, 0, &avg_id);

    if (h_avg == NULL || h_min_max == NULL) {
        printf("Failed creating threads.");
        return -1;
    }

    WaitForSingleObject(h_min_max, INFINITE);
    WaitForSingleObject(h_avg, INFINITE);
    CloseHandle(h_min_max);
    CloseHandle(h_avg);

    printf("Min: %d. Max: %d\n", min_max_param.min, min_max_param.max);
    printf("Avg: %f\n", avg_param.avg);

    for (int i = 0; i < arr.size; i++) {
        if (arr.data[i] == min_max_param.min || 
            arr.data[i] == min_max_param.max) {
            arr.data[i] = avg_param.avg;
        }
        printf("%d ", arr.data[i]);
    }

    printf("\nExecution ended\n");
    system("pause");
}