CC=gcc
CFLAGS=-g -c -Wall
SOURCES=stdior.c
OBJECTS=$(SOURCES:.c=.o)
EXE=stdior

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -rf $(OBJECTS) $(EXE)