@echo off

make test CC=gcc
rename test.exe test.gcc.exe
test.gcc > test.gcc.log

make test CC="zig cc"
rename test.exe test.zig.exe
test.zig > test.zig.log

make test CC=tcc
rename test.exe test.tcc.exe
test.tcc > test.tcc.log
