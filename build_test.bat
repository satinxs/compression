@echo off

make test CC=gcc
rename test.exe test.gcc.exe
make test CC="zig cc"
rename test test.zig.exe
make test CC=tcc
rename test test.tcc.exe
