CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
LIST=list
MAP=map

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(LIST).o $(MAP).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(LIST).o: $(LIST).c $(LIST).h
	$(CC) $(CFLAGS) $^ -c

$(MAP).o: $(MAP).c $(MAP).h
	$(CC) $(CFLAGS) $^ -c

pack:
	zip -FSr 312CA_DamianMihai-Robert_SD.zip README Makefile *.c *.h

clean:
	rm -f *.o tema2 *.h.gch
