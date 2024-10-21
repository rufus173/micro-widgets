CFLAGS=-g
LIBFOLDER=../../lib
EXECUTABLENAME=keybindr
$(EXECUTABLENAME) :  main.o input.o tray.o
	gcc $(CFLAGS) tray.o main.o input.o -o $(EXECUTABLENAME)
main.o : main.c $(LIBFOLDER)/input.h $(LIBFOLDER)/tray.h
	gcc $(CFLAGS) -c main.c
input.o : $(LIBFOLDER)/input.c
	gcc $(CFLAGS) -c $(LIBFOLDER)/input.c
tray.o : $(LIBFOLDER)/tray.c
	gcc $(CFLAGS) -c $(LIBFOLDER)/tray.c
set-id-$(EXECUTABLENAME) : $(EXECUTABLENAME)
	./set-id $(EXECUTABLENAME)
