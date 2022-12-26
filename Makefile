CFLAGS=-Wall -std=c99 -pthread
BINDIR=/usr/local/bin
FILEDIR=/usr/local/share
MANDIR=/usr/local/share/man/man1

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
LINK_LIBS=
else
LINK_LIBS=-lpigpiod_if2 -lm
endif

blinkt : main.c blinkt.c blinkt.h text.c text.h
	gcc $(CFLAGS) -o blinkt main.c blinkt.c text.c $(LINK_LIBS)

install : blinkt
	cp blinkt $(BINDIR)/
	chown :staff $(BINDIR)/blinkt
	chmod g+s $(BINDIR)/blinkt
	mkdir -p $(MANDIR)
	cp blinkt.1 $(MANDIR)/
	touch /usr/local/share/blinkt
	chmod 666 /usr/local/share/blinkt

clean :
	rm -f blinkt *.o

distclean :
	rm -f blinkt *.o $(BINDIR)/blinkt $(FILEDIR)/blinkt $(MANDIR)/blinkt.1
