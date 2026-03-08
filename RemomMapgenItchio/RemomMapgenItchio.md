
ReMoM::MapGen::itch.io

...a Zero-Effort Software for creating 'Master of Magic' (1994) scenerios...

not sure yet
but, maybe we have to get to WASM, for stich.io
also, pretty sure compiling C++ for WASM or Emscripten is straight-forward


The Emscripten package was installed here, by Chocolatey
C:\Users\jbalcomb\AppData\Local\emsdk
The Emscripten package also installed Node and Python.
EMSDK = C:/Users/jbalcomb/AppData/Local/emsdk 
EMSDK_NODE = C:\Users\jbalcomb\AppData\Local\emsdk\node\22.16.0_64bit\bin\node.exe
EMSDK_PYTHON = C:\Users\jbalcomb\AppData\Local\emsdk\python\3.13.3_64bit\python.exe

C:\Windows\system32>emcc -v
emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) 3.1.40 (5c27e79dd0a9c4e27ef2326841698cdd4f6b5784)
clang version 17.0.0 (https://github.com/llvm/llvm-project 2922e7cd9334797c24a317d41275f1258ef9ddd3)
Target: wasm32-unknown-emscripten
Thread model: posix
InstalledDir: C:\Users\jbalcomb\AppData\Local\emsdk\upstream\bin

C:\Users\jbalcomb\AppData\Local\emsdk\upstream\emscripten\emcc -v

C:\Users\jbalcomb\AppData\Local\emsdk\emcmdprompt.bat
- C:\Users\jbalcomb\AppData\Local\emsdk\emsdk_env.bat




# itch.io

https://jbalcomb.itch.io/maker-of-magic

Web Browser (HTML5/WebAssembly)
For games that can run directly in a web browser, use this method. 
Use Emscripten: This is a specific toolchain designed to compile C/C++ code into WebAssembly and JavaScript, which browsers can run. It also provides ports of libraries like SDL2 for web compatibility.
Generate Web Files: Emscripten will output several files, typically an index.html file, a .js file, and a .wasm file.
Package Your Web Build: Place all the generated web files into a single folder, and then create a .zip archive of that folder.
Upload to Itch.io:
When creating or editing your project, set "Kind of project" to "HTML".
Upload your ZIP file.
Ensure the index.html file is in the root of the archived folder, not inside a sub-folder within the zip.
Itch.io will automatically detect the HTML file and make the game playable in the browser on your project page. 

Advanced Upload Method: Butler
For developers who update their games frequently, the itch.io command-line tool Butler is highly recommended. It allows for fast, incremental updates by only pushing the changed parts of your build, rather than re-uploading the entire ZIP file each time.

