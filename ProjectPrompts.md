
As a classic, low-level C code, create a utility that parses C source code files, extracts all the functions names, and creates a text file with all of the function names for a given CMake target.






cmake --preset=MSVC26_x64-debug --workflow

cmake --preset=MSVC26_x64-debug --workflow MSVC26_x64-debug

cmake --build --preset MSVC26_x64-debug
cmake --workflow --preset MSVC26_x64-debug
cmake --workflow --preset MSVC26_x64-debug --target FuncExtract



$ cmake --list-presets
Available configure presets:

  "default-firstest"
  "Clang 17.0.3 (GNU CLI) - amd64 for MSVC 17.9.34701.34 (Visual Studio Community 2022 Release)" - Clang 17.0.3 (GNU CLI) - amd64 for MSVC 17.9.34701.34 (Visual Studio Community 2022 Release)
  "MSVC26_x64-debug"                                                                             - VS2026 amd64 Debug
  "MSVC26_x64-release"                                                                           - VS2026 amd64 Release
  "MSVC22_x64-debug"                                                                             - VS2022 amd64 Debug
  "MSVC22_x64-release"                                                                           - VS2022 amd64 Release
  "Ninja-MSVC_x64"                                                                               - Windows x64 Debug

jbalcomb@JHolcomb-W11 MINGW64 /c/STU/devel/STU-Extras (main)
$





Core logic for Parse_C_Source_File:

Read file line-by-line
Skip lines inside block comments (/* ... */) and string literals
For each line, call Looks_Like_Function_Definition which checks:
Line contains ( (parameter list)
The token before ( is an identifier (the function name)
The token before that is a return type (not a C keyword like if, while, for, switch, return)
The line or next line contains { (it's a definition, not a declaration ending in ;)
Extract function name and return type into FUNCTION_INFO



Verification
cmake -S . -B build — confirms build system generates without errors
cmake --build build --target FuncExtract — compiles the tool
Run: FuncExtract -t TootLib -c TootLib/CMakeLists.txt -o tootlib_functions.txt — extracts functions from TootLib sources
Run: FuncExtract -o c4lib_functions.txt C4lib/FC_File.C — direct file mode, should find FC_Load_File



where cmake 2>/dev/null || ls "C:/Program Files/CMake/bin/cmake.exe" 2>/dev/null || ls "C:/Program Files (x86)/CMake/bin/cmake.exe" 2>/dev/null || which cmake 2>/dev/null
