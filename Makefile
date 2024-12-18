all : battery-display shutdown-menu digital-clock
	@echo "All widgets compiled"
install : FORCE
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
	gcc -o digital-clock `pkg-config --cflags --libs gtk4` digital-clock.o
digital-clock.o : src/digital-clock.c
	gcc -c `pkg-config --cflags --libs gtk4` src/digital-clock.c
quick-launcher : FORCE
	cd src/quick-launcher/ ; qmake ; make quick-launcher && mv quick-launcher ../..
micro-taskbar : FORCE
	cd src/micro-taskbar ; qmake ; make micro-taskbar && mv micro-taskbar ../..
micro-runner : FORCE
	cd src/micro-runner ; qmake ; make micro-runner && mv micro-runner ../..
keybindr : FORCE
	make -C src/keybindr add-group-input ; cp -p src/keybindr/keybindr .
micro-manager : FORCE
	make -C src/micro-manager micro-manager ; cp src/micro-manager/micro-manager .
FORCE:
