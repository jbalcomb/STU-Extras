
ReMoM::MapGen::itch.io

...a Zero-Effort Software for creating 'Master of Magic' (1994) scenerios...

not sure yet
but, maybe we have to get to WASM, for stich.io
also, pretty sure compiling C++ for WASM or Emscripten is straight-forward



Claude's last known Todos list...
Update Todos
[x] Create CMakeLists.txt with SDL2, nlohmann/json, GTest
[x] Create MomConstants.hpp (game constants from MOM_DEF.H)
[x] Create MomWorld data layer (terrain grid)
[x] Create MomEntities (City/Unit/Wizard/Node/Lair structs)
[x] Create MomGamFile (.GAM binary read/write)
[x] Create Scenario and ScenarioIO (JSON format)
[x] Create EditorState and UndoStack
[x] Create Renderer (SDL2 window, camera, coords)
[x] Create MapRenderer (tile grid rendering)
[x] Create UIRenderer and UI panels
[x] Create main.cpp (app entry, main loop)
[x] Create unit tests (GamFile, Scenario, UndoStack)
[ ] Verify CMake configure and build





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

