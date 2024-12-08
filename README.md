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

- CMake: For building the project.
- A C++ Compiler: (e.g., GCC, Clang, or MSVC).
- qcustomplot library: Included in the repository; no need to install separately.

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
