# Build and Run Instructions

## Prerequisites

- **C++ Compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **CMake**: Version 3.10 or later
- **OpenGL**: Usually pre-installed on most systems
- **GLUT/FreeGLUT**: Window management and input handling

---

## macOS

### Install Dependencies
```bash
# Xcode command line tools (includes compiler and OpenGL)
xcode-select --install

# CMake (if not installed)
brew install cmake
```

GLUT is included with macOS as a system framework (no additional installation needed).

### Build
```bash
cd GLIDE
mkdir build
cd build
cmake ..
cmake --build .
```

### Run
```bash
./glide
```

---

## Linux (Ubuntu/Debian)

### Install Dependencies
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install freeglut3-dev
sudo apt install libgl1-mesa-dev
```

### Build
```bash
cd GLIDE
mkdir build
cd build
cmake ..
cmake --build .
```

### Run
```bash
./glide
```

---

## Windows

### Option A: Visual Studio

1. Install [CMake](https://cmake.org/download/)
2. Install [FreeGLUT](http://freeglut.sourceforge.net/) or use vcpkg:
   ```cmd
   vcpkg install freeglut:x64-windows
   ```
3. Generate Visual Studio project:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```
4. Open `build/GLIDE.sln` in Visual Studio
5. Build and run

### Option B: MinGW

1. Install MinGW-w64 and CMake
2. Install FreeGLUT for MinGW
3. Build:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   cmake --build .
   ```
4. Run: `glide.exe`

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `GLUT not found` | Install freeglut-dev (Linux) or check framework paths (macOS) |
| `OpenGL not found` | Install mesa-dev (Linux) or update graphics drivers |
| `GL_SILENCE_DEPRECATION` warnings on macOS | These are suppressed in CMakeLists.txt |
| Window doesn't appear | Check if X11/display server is running (Linux) |
| Compiler not found | Install build-essential (Linux) or Xcode CLI tools (macOS) |

---

## Quick One-Liner Build

```bash
# From the GLIDE directory:
mkdir -p build && cd build && cmake .. && cmake --build . && ./glide
```
