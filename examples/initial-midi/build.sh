#!/bin/bash
rm -rf build
set -e
mkdir -p build
cd build
cmake .. -DPICO_EXTRAS_ENABLE_PICOTOOL=0
make -j$(nproc)
