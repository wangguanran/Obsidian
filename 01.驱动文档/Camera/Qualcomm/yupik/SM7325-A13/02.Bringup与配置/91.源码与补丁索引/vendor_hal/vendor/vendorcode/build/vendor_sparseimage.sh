#!/bin/bash

PYTHON_BIN="${PYTHON_BIN:-python3}"
if ! command -v "${PYTHON_BIN}" >/dev/null 2>&1; then
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_BIN="python3"
    elif command -v python >/dev/null 2>&1; then
        PYTHON_BIN="python"
    else
        echo "ERROR: 未找到 python3 或 python" >&2
        exit 1
    fi
fi

cd vendor/vendorcode/build/sparseimage/

"${PYTHON_BIN}" checksparse.py -i rawprogram0_FFBM.xml -o rawprogram0_FFBM_split.xml
"${PYTHON_BIN}" checksparse.py -i rawprogram0.xml -o rawprogram0_split.xml

rm -rf *.bak
cd ../../../../
