CC=g++
CFLAGS=-std=c++11 -Wall -g -c
LDFLAGS=
SOURCES=main.cpp test.cpp interp.cpp compile.cpp r0.cpp x0.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

VPATH = rep

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)

