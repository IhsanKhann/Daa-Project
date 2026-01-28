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

