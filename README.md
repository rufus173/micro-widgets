
# micro-widgets

small utility widgets built with gtk4 and QT

# Requirements

version > Qt5 and version > GTK4 are both required, as well as the tool qmake.
The keybindr program also requires you be in group `input` to run
`micro-manager` requires Xlib to run

# Install

Some of these modules require gtk4 and some require qt to be installed for compilation.
all of the c files use gtk4 and all the c++ ones use qt.
You can use `make <widget>` to make any number of widgets and `make all` to make all of them. finaly `make install` will install all compiled widgets to `/usr/bin`. Keybindr may require you to relogin so the shell registers you as in group `input` to run without sudo.
Widgets to choose from:
- shutdown-menu (GTK)
- battery-display (GTK)
- digital-clock (GTK)
- quick-launcher (QT)
- micro-taskbar (QT)
- micro-runner (QT)
- keybindr (compile with sudo)
- micro-manager {it's a window manager for x} (Xlib)
