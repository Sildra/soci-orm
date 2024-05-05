#!/bin/sh

CXX_FLAGS="-Irapidjson/include"

LD_FLAGS="\
    -l:libsoci_sqlite3.a \
    -l:libsoci_core.a \
    -lsqlite3"

clang++ $CXX_FLAGS soci_orm/main.cpp -g $LD_FLAGS -o test.exe 
