CC=gcc

CFLAGS=-Iinclude
RELEASE_FLAGS=$(CFLAGS) -O3 -s -Wall -Wextra
DEBUG_FLAGS=$(CFLAGS) -O0 -g -Wall

EXT=
ifeq ($(OS), Windows_NT)
EXT=.exe
endif

build:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c $(DEBUG_FLAGS) -o compression$(EXT)

release:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c $(RELEASE_FLAGS) -o compression$(EXT)

profile:
	$(CC) main.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -O3 -Wall -Wextra -o compression$(EXT) -pg -Iinclude

test:
	$(CC) test.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -Include $(RELEASE_FLAGS) -o test$(EXT)

test-debug:
	$(CC) test.c command_line.c lib/hash.c lib/bit_stream.c lib/lzss.c lib/rolz.c -Include $(DEBUG_FLAGS) -o test$(EXT)

clean:
	rm -rf *.exe *.pdb
