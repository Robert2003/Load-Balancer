CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
LIST=list
MAP=map

.PHONY: build clean

build: ld_balancer

ld_balancer: main.o load_balancer/$(LOAD).o load_balancer/$(SERVER).o my_lib/$(LIST).o my_lib/$(MAP).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

(SERVER).o: load_balancer/$(SERVER).c load_balancer/$(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: load_balancer/$(LOAD).c load_balancer/$(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(LIST).o: my_lib/$(LIST).c my_lib/$(LIST).h
	$(CC) $(CFLAGS) $^ -c

$(MAP).o: my_lib/$(MAP).c my_lib/$(MAP).h
	$(CC) $(CFLAGS) $^ -c

pack:
	zip -FSr Load_Balancer.zip README Makefile *.c *.h

clean:
	rm -f */*.o ld_balancer */*.h.gch main.o
