CFLAGS=-Wall -std=c99
BINDIR=/usr/local/bin
MANDIR=/usr/local/share/man/man1

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
LINK_LIBS=
else
LINK_LIBS=-lwiringPi -lm
endif

blinkt : main.c blinkt.c blinkt.h text.c text.h
	gcc $(CFLAGS) -o blinkt main.c blinkt.c text.c $(LINK_LIBS)

install : blinkt
	cp blinkt $(BINDIR)/
	mkdir -p $(MANDIR)
	cp blinkt.1 $(MANDIR)/

clean :
	rm -f blinkt *.o

distclean :
	rm -f blinkt *.o $(BINDIR)/blinkt $(MANDIR)/blinkt.1
