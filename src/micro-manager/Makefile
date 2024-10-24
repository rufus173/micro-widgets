CFLAGS=
micro-manager : main.o
	gcc main.o `pkg-config --libs x11` -o micro-manager
main.o : main.c
	gcc $(CFLAGS) -c main.c
debug-flags :
	$(eval CFLAGS += -g)
micro-manager-debug : debug-flags micro-manager
	
