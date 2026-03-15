# PC Core Project

This project is a small C++ log-monitoring program for the Smart Connected Devices coursework. It watches CSV-style accelerometer output from the STM32 side, parses new samples, and prints the values to the PC console in real time.

## File Overview

### Root files

- `CMakeLists.txt` sets up the CMake build, lists the source files, and enables compiler warning flags.
- `README.md` gives a short overview of the project, how to build it, and what each file is for.

### Header files (`include/`)

- `include/LogMonitorApp.hpp` declares the main application class and its `run()` entry point.
- `include/FileWatcher.hpp` declares the file-watching helper that waits for files, opens them, and polls for new lines.
- `include/CsvParser.hpp` declares the CSV parser that converts a line of text into a `Sample`.
- `include/Sample.hpp` defines the `Sample` struct used to store one parsed accelerometer reading.
- `include/SerialOutputPrinter.hpp` declares the printer used to format and display parsed samples.

### Source files (`src/`)

- `src/main.cpp` is the program entry point and starts `LogMonitorApp`.
- `src/LogMonitorApp.cpp` contains the main program logic, command-line option handling, default path detection, file or directory watch mode, and log-file switching.
- `src/FileWatcher.cpp` implements the logic for waiting on a file, opening it at the start or end, and reading only complete new lines.
- `src/CsvParser.cpp` trims and splits CSV lines, skips headers or invalid rows, and converts valid rows into `Sample` values.
- `src/SerialOutputPrinter.cpp` prints each parsed sample to the console in a readable format.

### Build output folders

- `build/` contains generated build files, the compiled executable, and some test/output logs from local runs.
- `build-auto/` contains an additional generated build tree, likely from an IDE or automated CMake workflow.
