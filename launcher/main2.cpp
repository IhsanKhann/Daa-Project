#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <thread>

using namespace std;

// ===========================
// Helper: Measure CPU usage of a core
// ===========================
float getCoreUsage(int core) {
    FILETIME idleTime, kernelTime, userTime;
    static FILETIME prevIdle = {}, prevKernel = {}, prevUser = {};
    
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0f;

    ULONGLONG idle = (((ULONGLONG)idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
    ULONGLONG kernel = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
    ULONGLONG user = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;

    ULONGLONG prevIdleVal = (((ULONGLONG)prevIdle.dwHighDateTime) << 32) | prevIdle.dwLowDateTime;
    ULONGLONG prevKernelVal = (((ULONGLONG)prevKernel.dwHighDateTime) << 32) | prevKernel.dwLowDateTime;
    ULONGLONG prevUserVal = (((ULONGLONG)prevUser.dwHighDateTime) << 32) | prevUser.dwLowDateTime;

    ULONGLONG idleDiff = idle - prevIdleVal;
    ULONGLONG totalDiff = (kernel + user) - (prevKernelVal + prevUserVal);

    prevIdle = idleTime;
    prevKernel = kernelTime;
    prevUser = userTime;

    if (totalDiff == 0) return 0.0f;
    float usage = 100.0f * (1.0f - ((float)idleDiff / totalDiff));
    return usage;
}

// ===========================
// Run a child algorithm exe
// ===========================
bool runAlgorithm(const string& exe, int inputSize, int core, long long& timeResult)
{
    string cmd = exe + " " + to_string(inputSize);

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE readPipe = NULL, writePipe = NULL;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) return false;
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);  
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    vector<char> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back('\0');

    if (!CreateProcessA(NULL, cmdBuffer.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        cerr << "Failed to create process: " << exe << " Error: " << GetLastError() << "\n";
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return false;
    }

    if (core >= 0) {
        DWORD_PTR mask = (1ULL << core);
        SetProcessAffinityMask(pi.hProcess, mask);
    }

    CloseHandle(writePipe);

    // Wait for child with timeout
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000); 
    if (waitResult == WAIT_TIMEOUT) {
        cerr << "Process timed out: " << exe << "\n";
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(readPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (exitCode != 0) cerr << "Process exited with code " << exitCode << ": " << exe << "\n";

    // Read all output
    string output;
    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    CloseHandle(readPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Trim
    output.erase(0, output.find_first_not_of(" \t\n\r"));
    output.erase(output.find_last_not_of(" \t\n\r") + 1);

    if (output.empty()) {
        cerr << "No output from: " << exe << "\n";
        timeResult = 0;
        return false;
    }

    try { timeResult = stoll(output); return true; }
    catch (...) { timeResult = 0; return false; }
}

// ===========================
// Wait until core usage is below threshold
// ===========================
void waitForCoreIdle(int core, float maxUsagePercent = 20.0f) {
    float usage = getCoreUsage(core);
    while (usage > maxUsagePercent) {
        this_thread::sleep_for(chrono::milliseconds(50));
        usage = getCoreUsage(core);
    }
}

// ===========================
// Main launcher
// ===========================
int main()
{
    filesystem::create_directories("results");

    vector<string> algos = {"build/linear.exe", "build/binary.exe"};
    vector<string> names = {"Linear Search", "Binary Search"};
    vector<int> sizes = {1000, 5000, 10000, 50000, 100000};

    ofstream file("results/results.csv");
    file << "Algorithm,InputSize,Time(ns)\n";

    cout << "\n=== AUTOMATED SEARCH COMPARISON ===\n\n";

    int testCore = 0;
    float allowedIdlePercent = 20.0f; // only run if core usage < 20% (i.e., 80% free for fairness)

    for (size_t i = 0; i < algos.size(); i++) {
        if (!filesystem::exists(algos[i])) {
            cerr << "Executable not found: " << algos[i] << "\n";
            continue;
        }

        cout << names[i] << ":\n";

        for (int n : sizes) {
            // Wait until core is lightly loaded
            waitForCoreIdle(testCore, allowedIdlePercent);

            long long time = 0;
            bool success = runAlgorithm(algos[i], n, testCore, time);

            if (!success) {
                cerr << "  Failed to run N=" << n << "\n";
                continue;
            }

            file << names[i] << "," << n << "," << time << "\n";
            cout << "  N=" << n << " | " << time << " ns (" << (time/1000.0) << " μs)\n";
        }
        cout << "----------------------------\n";
    }

    file.close();
    cout << "\nResults written to results/results.csv\n";
    cout << "\nPress Enter to exit...";
    cin.get();
    return 0;
}

/*
Fair CPU usage:
waitForCoreIdle() checks core usage and only runs the test if CPU load is below threshold (default 20%).
This ensures all algorithms run under roughly the same conditions.
CPU affinity:
Child processes are pinned to a single core (testCore = 0) for consistent benchmarking.
Robust process handling:
Read entire stdout pipe, check exit code, timeout handling.
Folder and file handling:
Automatically creates results/ folder.
Safe CSV output.
Consistency:
Every test waits for “fair CPU availability” before starting → all comparisons are under similar conditions.
*/