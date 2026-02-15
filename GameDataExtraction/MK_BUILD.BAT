@ECHO OFF
REM Microsoft C/C++ Optimizing Compiler (v19.28.29337) (x64)

if not exist mkbuild (mkdir mkbuild)

pushd mkbuild

REM @SET SRC_PROJECT=../Spell_Data_Table.C
@SET SRC_PROJECT=../Bldg_Data_Table.C

REM cl /c %SRC_PROJECT%

REM lib Spell_Data_Table.obj /OUT:Spell_Data_Table.lib

REM cl /DUNDEFINE_UNICODE /DSTU_DEBUG /D__WIN32__ /RTCs /RTCu /W1 /wd4068 /Zc:strictStrings -FC /Fe: ReMoM.exe -I ..\src\STU -I ..\src -Zi %WIN_PFL% %SRC_MOM% user32.lib gdi32.lib winmm.lib MOX.lib STU.lib
REM cl /MT /Fe: Spell_Data_Table.exe Spell_Data_Table.lib /link /SUBSYSTEM:CONSOLE /entry:main
cl %SRC_PROJECT%

popd
