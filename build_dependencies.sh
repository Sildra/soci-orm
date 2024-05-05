#!/bin/bash

mkdir -p build/sqlite
cd build/sqlite
../../sqlite/configure
make
make install
cd -


mkdir -p build/soci
cmake -G "Unix Makefiles" ../../soci
make
make install
