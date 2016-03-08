CFLAGS = -Wall -g

psort: psort.o
	gcc $(CFLAGS) -o psort psort.o

psort.o: psort.c psort.h
	gcc $(CFLAGS) -c psort.c
