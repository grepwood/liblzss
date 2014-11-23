CC=gcc
CFLAGS=-O2
WFLAGS=-Wall -Wextra -pedantic 
LFLAGS=-shared -fPIC
PREFIX=/usr/local
IFLAGS=-I.

all: object examples lib

object:
	$(CC) $(CFLAGS) $(WFLAGS) $(LFLAGS) -c lzss.c

lib:
	$(CC) $(CFLAGS) $(WFLAGS) $(LFLAGS) lzss.o -o liblzss.so

install:
	cp liblzss.so $(PREFIX)/lib
	cp lzss.h $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/liblzss.so
	rm -f $(PREFIX)/include/lzss.h

examples: 
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/comp-ff.c -o comp-ff
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/comp-mf.c -o comp-mf
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/comp-fm.c -o comp-fm
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/comp-mm.c -o comp-mm
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/decomp-ff.c -o decomp-ff
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/decomp-mf.c -o decomp-mf
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/decomp-fm.c -o decomp-fm
	$(CC) $(CFLAGS) $(WFLAGS) $(IFLAGS) lzss.o test/decomp-mm.c -o decomp-mm

clean:
	rm -f lzss.o liblzss.so {de,}comp-{ff,mf,fm,mm}
