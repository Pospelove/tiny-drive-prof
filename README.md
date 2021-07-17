# tiny-drive-prof

Disk space profiler tool written in C++.

## Features

* Simple and responsive UI.
* Multithreaded search. Uses `std::thread::hardware_concurrency()` to determine number of threads.
* Real-time filesystem events handling.
* For devs: Hot reload based on `cr.h`.

## Installation

* Windows 7 or higher is required *([Windows 10](https://www.microsoft.com/en-us/software-download/windows10) is recommended)*
* Download [prebuilt version](https://github.com/Pospelove/tiny-drive-prof/releases/).
* Extract an archive and run `tiny-drive-prof.exe`.

## Building from Sources

### Prerequisites

Before your start make sure that your system meets the conditions:

* Windows 7 or higher *([Windows 10](https://www.microsoft.com/en-us/software-download/windows10) is recommended)*
* [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
* [CMake 3.19.1-3.20](https://cmake.org/download/) (3.21 wouldn't work)

### Configuring and Building

1. Clone the repo, including submodules
   ```sh
   git clone https://github.com/Pospelove/tiny-drive-prof.git
   cd tiny-drive-prof
   git submodule init
   git submodule update
   ```
2. Make a build directory (used for project files, cache, artifacts, etc)
   ```sh
   mkdir build
   ```
3. Generate project files with CMake. Dependencies would be installed automatically
   ```sh
   cd build
   cmake ..
   ```
4. Open `build/tiny-drive-prof.sln` with Visual Studio, then `Build -> Build Solution`.
