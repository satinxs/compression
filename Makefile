CC=gcc

CFLAGS=-Iinclude
RELEASE_FLAGS=$(CFLAGS) -O3 -s -Wall -Wextra
DEBUG_FLAGS=$(CFLAGS) -O0 -g3 -Wall -gdwarf-2 -ggdb -fno-omit-frame-pointer

ifeq ($(OS), Windows_NT)
OUTPUT=compression.exe
else
OUTPUT=compression
endif

build:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c $(DEBUG_FLAGS) -o $(OUTPUT)

profile:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -Iinclude -O3 -Wall -fno-pie -pg -o $(OUTPUT)

test:
	$(CC) test.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -Include $(RELEASE_FLAGS) -o test

test-debug:
	$(CC) test.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -Include $(DEBUG_FLAGS) -o test

release:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c $(RELEASE_FLAGS) -o $(OUTPUT)

clean:
	rm -rf *.exe *.pdb
