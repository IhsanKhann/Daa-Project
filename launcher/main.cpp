#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

using namespace std;

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
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        cerr << "Failed to create pipe\n";
        return false;
    }
    
    // Ensure the read handle is not inherited
    if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
        cerr << "Failed to set handle information\n";
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return false;
    }

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);  // Keep stderr visible
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    // Create a modifiable copy of the command string
    vector<char> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back('\0');

    if (!CreateProcessA(NULL, cmdBuffer.data(),
                       NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        cerr << "Failed to create process: " << exe << " Error: " << GetLastError() << "\n";
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return false;
    }

    // Set CPU affinity (optional - pin to specific core)
    if (core >= 0) {
        DWORD_PTR mask = (1ULL << core);
        SetProcessAffinityMask(pi.hProcess, mask);
    }

    // IMPORTANT: Close the write end of the pipe in the parent process
    // This ensures that when the child exits, ReadFile will return
    CloseHandle(writePipe);

    // Wait for child to finish (with timeout to prevent hanging)
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000); // 30 second timeout
    if (waitResult == WAIT_TIMEOUT) {
        cerr << "Process timed out: " << exe << "\n";
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(readPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    // Check exit code
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (exitCode != 0) {
        cerr << "Process exited with error code " << exitCode << ": " << exe << "\n";
    }

    // Read all output from child
    string output;
    char buffer[4096];
    DWORD bytesRead;
    
    while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    // Close handles
    CloseHandle(readPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Trim whitespace from output
    output.erase(0, output.find_first_not_of(" \t\n\r"));
    output.erase(output.find_last_not_of(" \t\n\r") + 1);

    // Convert output safely
    if (output.empty()) {
        cerr << "No output received from: " << exe << "\n";
        timeResult = 0;
        return false;
    }

    try {
        timeResult = stoll(output);
        return true;
    } catch (const invalid_argument& e) {
        cerr << "Invalid output from " << exe << ": '" << output << "'\n";
        timeResult = 0;
        return false;
    } catch (const out_of_range& e) {
        cerr << "Output out of range from " << exe << ": '" << output << "'\n";
        timeResult = 0;
        return false;
    }
}

// ===========================
// Main launcher
// ===========================
int main()
{
    // Ensure results folder exists
    filesystem::create_directories("results");

    vector<string> algos = {
        "build/linear.exe",
        "build/binary.exe"
    };

    vector<string> names = {
        "Linear Search",
        "Binary Search"
    };

    // dynamic sizes to test..
    vector<int> sizes = {1000, 5000, 10000, 50000, 100000};

    ofstream file("results/results.csv");
    if (!file.is_open()) {
        cerr << "Failed to open results file\n";
        return 1;
    }

    file << "Algorithm,InputSize,Time(ns)\n";

    cout << "\n=== AUTOMATED SEARCH COMPARISON ===\n\n";

    for (size_t i = 0; i < algos.size(); i++)
    {
        // Check if executable exists
        if (!filesystem::exists(algos[i])) {
            cerr << "ERROR: Executable not found: " << algos[i] << "\n";
            continue;
        }

        cout << names[i] << ":\n";
        
        for (int n : sizes)
        {
            long long time = 0;
            bool success = runAlgorithm(algos[i], n, 0, time);

            if (!success) {
                cerr << "  Failed to run with N=" << n << "\n";
                continue;
            }

            file << names[i] << "," << n << "," << time << "\n";

            cout << "  N=" << n 
                 << " | " << time << " ns"
                 << " (" << (time / 1000.0) << " μs)\n";
        }
        cout << "----------------------------\n";
    }

    file.close();
    cout << "\nResults written to results/results.csv\n";
    cout << "\nPress Enter to exit...";
    cin.get();
    return 0;
}

// file has input sizes
// file has algorithm names
// runs each algorithm exe with each input size, compare them..
// captures output time in ns
// writes results to CSV file
// displays progress on console
