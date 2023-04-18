# GarblingGates
Gabling Gates Master Thesis
By Aske, Simon and Svend

## Installation
The project uses C++ 17 and Cmake to build our project. The libraries used for this project are:
- Openssl 3.x
- Boost

This project also takes advantage of the [SSE2](https://en.wikipedia.org/wiki/SSE2) instruction set, and thus all packages has to be compiled for x64.

### Windows
Our setup on Windows is as follows: Clion using the MSVC C++ compiler.
The libraries mentioned above can be installed on windows using the [vcpkg](https://vcpkg.io/en/) package manager for C++ libraries.
1. Download MSVC compiler from: [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/)
2. Download Clion: [CLION](https://www.jetbrains.com/clion/)
3. Install Cmake: [Cmake](https://cmake.org/)
4. Clone vcpkg and run the bootstrap script:
```cmd
> git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
```
5. Install the aforementioned libraries using(since we use SSE2 instructions this has to be for x64):
```cmd
> .\vcpkg\vcpkg install [package name]:x64-windows
```
6. Intergrate the libaries into the compiler:
```cmd
> .\vcpkg\vcpkg integrate install
```
This returns a string on the form: `-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake`
7. Add the -DCMAKE... line to clion's section for cmake options(Build, Execution, Deployment > CMake). Add the line under `CMake options`.

### Linux
penguin magic

### Running our program
