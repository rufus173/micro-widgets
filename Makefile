all : battery-display shutdown-menu
	@echo "All widgets compiled"
install : README.md
	@./install
shutdown-menu : shutdown-menu.o
	gcc shutdown-menu.o -o shutdown-menu `pkg-config --cflags --libs gtk4` 
shutdown-menu.o : src/shutdown-menu.c
	gcc -c src/shutdown-menu.c `pkg-config --cflags --libs gtk4`
battery-display : battery-display.o
	gcc -o battery-display `pkg-config --cflags --libs gtk4` battery-display.o
battery-display.o : src/battery-display.c
	gcc -c `pkg-config --cflags --libs gtk4` src/battery-display.c
digital-clock : digital-clock.o
	gcc -o `pkg-config --cflags --libs gtk4` digital-clock.o
digital-clock.o : src/digital-clock.c
	gcc -c `pkg-config --cflags --libs gtk4` src/digital-clock.c

