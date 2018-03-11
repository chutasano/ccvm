CC=g++
CFLAGS=-std=c++11 -Wall -g -c
LDFLAGS=
SOURCES=main.cpp test.cpp interp.cpp r0.cpp x0.cpp c0.cpp x0s.cpp
OBJECTS=$(SOURCES:.cpp=.o) compile.o
EXECUTABLE=main
ABSPATH=$(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

VPATH = rep

all: $(SOURCES) compile.o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

compile.o: compile.cpp compile.h
	$(CC) $(CFLAGS) -DRUNTIME=$(ABSPATH)/runtime/ $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)

