#include <iostream>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace chrono;

int binarySearch(int* arr, int n, int key) {
    int left = 0;
    int right = n - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == key)
            return mid;
        else if (arr[mid] < key)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: binary.exe <array_size>" << endl;
        return 1;
    }

    int n = atoi(argv[1]);
    int repetitions = 1000;  // Number of times to repeat the search

    // Create sorted array (binary search requires sorted data)
    int* arr = new int[n];
    for (int i = 0; i < n; i++)
        arr[i] = i * 2;

    // Search for element in the middle
    int key = arr[n / 2];

    auto start = high_resolution_clock::now();

    // Repeat the search multiple times for better timing accuracy
    for (int rep = 0; rep < repetitions; rep++) {
        binarySearch(arr, n, key);
    }

    auto end = high_resolution_clock::now();

    long long total = duration_cast<nanoseconds>(end - start).count();
    long long average = total / repetitions;
    
    // Output ONLY the number - no extra text
    cout << average << flush;

    delete[] arr;
    return 0;
}