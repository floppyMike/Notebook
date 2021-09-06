# Notebook
A extendable notebook designed for speed and simplicity.

## Building
For all systems it starts of with either directly downloading the source files from here or using the command line.
```
git clone --recurse-submodules https://github.com/floppyMike/Notebook.git
```
If you don't use the above command then make sure you also download all submodules inside ./extern by manually downloading them.
Current submodules are:
1. [floppyMike/CustomLibrary](https://github.com/floppyMike/CustomLibrary)

### Windows
For this build I'll be using [Visual Studio Build Tools 2019](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) to compile the project. This should be installed when you're using Visual Studio.
Make sure you have the component `Desktop development with C++` installed.
1. Navigate with the `Developer Command Prompt` to the project directory.
```
cd <path>
```
2. Create a new directory called `build`
```
mkdir build && cd build
```
3. Generate the project and compile the binary
```
cmake .. && cmake --build . --config Release
```
4. The binary will be located at `.\build\Release\Notetaker.exe`. But to execute it you'll also need to place the dll files from `extern` into the same directory as the binary.

### Linux
Before you start you should have the following programms installed:
```
sudo apt install gcc cmake
```
1. Navigate with the terminal to the project directory.
```
cd <path>
```
2. Create a new directory called `build`
```
mkdir build && cd build
```
3. Generate the project and compile the binary
```
cmake .. && cmake --build . --config Release
```
4. The binary will be located at `./build/Notetaker`
