# GarblingGates
Gabling Gates Master Thesis
By Aske, Simon and Svend

## Installation
The project uses C++ 17 and Cmake to build our project. The libraries used for this project are:
- Openssl 3.x
- Boost

This project also takes advantage of the [SSE2](https://en.wikipedia.org/wiki/SSE2) instruction set, and thus all packages has to be compiled for x64. Specifically it uses instructions from SSE4.1 to set up for the CLMUL instruction.

### Windows
Our setup on Windows is as follows: Clion using the MSVC C++ compiler.
The libraries mentioned above can be installed on windows using the [vcpkg](https://vcpkg.io/en/) package manager for C++ libraries.
1. Download MSVC compiler from: [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/)
2. Download Clion: [CLION](https://www.jetbrains.com/clion/)
3. Download Cmake: [Cmake](https://cmake.org/)
4. Clone vcpkg and run the bootstrap script:
```cmd
> git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
```
5. Install the aforementioned libraries using(since we use SSE2 instructions this has to be for x64):
```cmd
> .\vcpkg\vcpkg install [package name]:x64-windows
```
6. Integrate the libraries into the compiler:
```cmd
> .\vcpkg\vcpkg integrate install
```
This returns a string on the form: `-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake`.

7. Add the -DCMAKE... line to Clion's section for cmake options(Build, Execution, Deployment > CMake). Add the line under `CMake options`.

### Linux
Our setup on linux is as follows: Ubuntu using Clion with the Clang compiler.
The libraries mentioned above can be installed using the command line.
1. Install Boost
```cmd
> sudo apt install libboost-all-dev
```
2. Install openssl 3. We followed this [OpenSSL guide](https://nextgentips.com/2022/03/23/how-to-install-openssl-3-on-ubuntu-20-04/) and it worked for ubuntu 22 aswell:
   
3. Install Clang
```cmd
> sudo apt install clang
```
4. Choose Clang as a compiler by selecting it in the ToolChains menu(Build, Execution, Deployment > Toolchains). Our installation directory was `/bin/clang-cpp-xx` where xx is the verison number.

5. Add the -DCMAKE... line to Clion's section for cmake options(Build, Execution, Deployment > CMake). Add the following under `CMake options`.
```cmd
-DCMAKE_C_COMPILER=clang
-DCMAKE_CXX_COMPILER=clang++
-DCMAKE_INCLUDE_PATH=/usr/local/ssl/include
-DOPENSSL_ROOT_DIR=/usr/local/ssl
-DOPENSSL_CRYPTO_LIBRARY=/usr/local/ssl/lib64/libcrypto.so
```
### Running our program
Our program as two executable targets defined:
- The Test target, which runs all our tests.
- GarblingGates, which runs the test of garbling the AES_128 circuit 1000 times for each scheme and recording the time it takes.
