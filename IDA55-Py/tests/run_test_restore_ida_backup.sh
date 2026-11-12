#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
python3 -m unittest tests.test_restore_ida_backup
