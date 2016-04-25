OWNER=bin
GROUP=bin
BINDIR=/usr/local/bin

huffpacman: huffpacman.o
	$(CC) $(LDFLAGS) $^ -o $@

CFLAGS = -O2 -Wall -Wextra

clean:
	-rm -f huffpacman.o huffpacman

install: huffpacman
	install -c -o $(OWNER) -g $(GROUP) -m 755 huffpacman $(DESTDIR)$(BINDIR)

