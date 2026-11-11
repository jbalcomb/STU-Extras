#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
if [[ -x /c/python314/python.exe ]]; then
  PYTHON_EXE=/c/python314/python.exe
elif [[ -x /mnt/c/python314/python.exe ]]; then
  PYTHON_EXE=/mnt/c/python314/python.exe
else
  PYTHON_EXE=python3
fi

"$PYTHON_EXE" ida_proc_data_export/ida55_export_function_data.py \
  --copy \
  --timeout-seconds 0 \
  --python "$PYTHON_EXE"

"$PYTHON_EXE" ida_proc_data_export/ida55_function_dump.py \
  in/MAGIC.idb.ida55-functions.txt \
  -o in/MAGIC.idb.ida55-functions.json

"$PYTHON_EXE" ida_proc_data_export/ida55_function_dump.py \
  in/WIZARDS.idb.ida55-functions.txt \
  -o in/WIZARDS.idb.ida55-functions.json

"$PYTHON_EXE" ida_proc_status_sync/ida_proc_status_sync.py
