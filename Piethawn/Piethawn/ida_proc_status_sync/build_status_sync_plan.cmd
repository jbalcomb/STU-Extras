@echo off
setlocal

cd /d "%~dp0\.."

C:\python314\python.exe ida_proc_data_export\ida55_export_function_data.py ^
  --copy ^
  --timeout-seconds 0 ^
  --python C:\python314\python.exe
if errorlevel 1 exit /b %errorlevel%

C:\python314\python.exe ida_proc_data_export\ida55_function_dump.py ^
  in\MAGIC.idb.ida55-functions.txt ^
  -o in\MAGIC.idb.ida55-functions.json
if errorlevel 1 exit /b %errorlevel%

C:\python314\python.exe ida_proc_data_export\ida55_function_dump.py ^
  in\WIZARDS.idb.ida55-functions.txt ^
  -o in\WIZARDS.idb.ida55-functions.json
if errorlevel 1 exit /b %errorlevel%

C:\python314\python.exe ida_proc_status_sync\ida_proc_status_sync.py
