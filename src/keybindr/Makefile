CFLAGS=-g
keybindr :  main.o input.o
	gcc $(CFLAGS) main.o input.o -o keybindr
main.o : main.c ../../lib/input.h
	gcc $(CFLAGS) -c main.c
input.o : ../../lib/input.c
	gcc $(CFLAGS) -c ../../lib/input.c
