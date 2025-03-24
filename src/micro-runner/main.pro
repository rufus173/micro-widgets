TEMPLATE = app
HEADERS += main.h
SOURCES += main.cpp tray.c applications.c config_file_lib.c
TARGET = micro-runner
QT = core gui widgets
QT += widgets
CONFIG += debug
