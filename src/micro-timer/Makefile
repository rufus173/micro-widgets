CFLAGS=-g `pkg-config --cflags gtk4`
LIBS=`pkg-config --libs gtk4`#-fsanitize=address
CC=gcc

OBJ_LIST=main.o

micro-timer : $(OBJ_LIST)
	$(CC) $^ -o $@ $(LIBS)
