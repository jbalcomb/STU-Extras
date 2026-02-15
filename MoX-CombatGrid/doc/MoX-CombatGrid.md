


SDL2 + ImGui + ReMoM + MoX

CMake
CLang

manually add SDL2
manually add ImGui

uses SDL2.dll in build directory
compiles with ImGui sources

ReMoM as a git submodule

To also initialize, fetch and checkout any nested submodules, you can use the foolproof 
git submodule update --init --recursive

git config -f .gitmodules submodule.ReMoM.branch main


git submodule add https://github.com/jbalcomb/ReMoM.git
git push origin master

git fetch
git merge origin/main

git checkout main
git submodule update --remote --merge
