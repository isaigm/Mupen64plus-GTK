CFLAGS = -Wall -pedantic -rdynamic
LDFLAGS = -lzip `pkg-config --cflags --libs gtk+-3.0` `pkg-config gtk+-3.0 --libs`

Mupen64: main.c
	@gcc main.c -o main $(CFLAGS) $(LDFLAGS)


