NOTE: Unfortunately, the original version-controlled repository was too large to upload to GitHub because I had accidentally committed several hundred massive binary game datasets rather than adding them to `.gitignore`. Instead, I have included [COMMIT_HISTORY.txt](https://github.com/SGD2718/Computer-Othello/blob/main/COMMIT_HISTORY.txt), which contains all the commit messages from the original.

---

# Computer Othello Bot

## How to Download and Build

### 1. Clone the Repository
Start by cloning the repository from GitHub to your local machine:

```bash
git clone https://github.com/SGD2718/Computer-Othello.git
cd Computer-Othello
```

### 2. Dependencies
Ensure you have the following dependencies installed:

- CMake: For building the project. Download it here: https://cmake.org/download/
- A C++ Compiler:
  - GCC (Linux/Mac users): https://gcc.gnu.org/install/
  - Clang (Linux/Mac users): https://clang.llvm.org/get_started.html
  - MSVC (Windows users, included with Visual Studio): https://visualstudio.microsoft.com/
- Qt GUI Library: Required for GUI-related components. Download it here: https://www.qt.io/download

### 3. File Structure
The repository includes the following:

- `CMakeLists.txt`: Build configuration for the project.
- `lib/QCustomPlot`: Library folder for custom plotting.
- `assets`: Folder containing necessary game assets.
- `src`: Folder containing all source code.

### 4. Build the Project
Follow these steps to build the project:

1. Create a `build` directory:

```bash
mkdir build
cd build
```

2. Run `cmake` to configure the build:

```bash
cmake ..
```

3. Build the project:

```bash
cmake --build .
```

### 5. Run the Program
After building, you can run the program directly from the `build` directory:

```bash
./Othello
```

### 6. Notes
- Make sure the `assets` folder remains in the same directory as the executable; it contains essential files for the bot to function.
- If you encounter issues, check that your compiler supports C++17 or higher.
