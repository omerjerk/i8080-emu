CC=gcc
PACKAGE = `pkg-config --cflags --libs gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

build: main.c
	$(CC) $(PACKAGE) -o 8080 main.c $(LIBS)