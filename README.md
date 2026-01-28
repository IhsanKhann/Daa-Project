# DAA Benchmark Project

## Overview

This project is designed to **benchmark algorithms** (like Linear Search, Binary Search) with **fair and consistent conditions**.  
The system pins algorithm executions to a **specific CPU core** and waits for the core to be lightly loaded before running, ensuring that all tests are comparable.

You can **add your own algorithms** in C++ and compare their performance across different input sizes.

---

## Features

- Executes algorithm `.exe` files with specific input sizes.
- Measures execution time in **nanoseconds** and writes results to CSV.
- Pins each algorithm execution to a specific CPU core for consistency.
- Waits for CPU usage to be below a threshold before running tests.
- Automatically generates results in `results/results.csv`.
- Supports multiple algorithms and input sizes.
- Fully configurable for cores, input sizes, and algorithms.

---

## Project Structure

<<<<<<< HEAD
=======
DAA/
├── algorithms/ # Your algorithm C++ source files
├── build/ # Compiled executables
├── launcher/ # Launcher code (runs algorithms)
├── plots/ # Optional: plots generated from CSV
├── results/ # Benchmark results (CSV)
├── tests/ # Test scripts or additional testing code
├── build1.ps # PowerShell build script
├── build2.ps # Alternate PowerShell build script
├── README.md # This file
├── .gitignore # Git ignore file


---

## Required VSCode Extensions

If using VSCode:

- **C/C++ by Microsoft** (IntelliSense, debugging)
- **Code Runner** (optional, run C++ programs)
- **CMake Tools** (optional, if using CMake)

If **not using VSCode**, you can compile and run using **PowerShell or terminal** (see Implementation Guide below).

---

## License

MIT License. Feel free to use and modify.
>>>>>>> d8a3c3ba6e677b93fc8d176e303c06da92ae16dd
