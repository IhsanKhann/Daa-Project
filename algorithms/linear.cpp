#include <iostream>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace chrono;

void linearSearch(int* arr, int n, int key) {
    for (int i = 0; i < n; i++) {
        if (arr[i] == key)
            return;
    }
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
        linearSearch(arr, n, key);
    }

    auto end = high_resolution_clock::now();

    long long total = duration_cast<nanoseconds>(end - start).count();
    long long average = total / repetitions;
    
    // Output ONLY the number - no extra text
    cout << average << flush;

    delete[] arr;
    return 0;
}
