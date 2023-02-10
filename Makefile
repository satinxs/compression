CC=gcc

CFLAGS=-Iinclude
RELEASE_FLAGS=$(CFLAGS) -O3 -s -Wall
DEBUG_FLAGS=$(CFLAGS) -O0 -g -Wall

ifeq ($(OS), Windows_NT)
OUTPUT=test.exe
else
OUTPUT=test
endif

build:
	$(CC) main.c lib/bit_stream.c lib/lzss.c $(DEBUG_FLAGS) -o $(OUTPUT)

release:
	$(CC) main.c lib/bit_stream.c lib/lzss.c $(RELEASE_FLAGS) -o $(OUTPUT)

clean:
	rm -rf *.exe *.pdb
