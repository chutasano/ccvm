all: main

main: main.o test.o interp.o compile.o
	g++ -o main main.o test.o interp.o compile.o

main.o: main.cpp test.h interp.h compile.h r0.h
	g++ -std=c++11 -Wall -g -c main.cpp -o main.o

test.o: test.cpp test.h interp.h compile.h r0.h
	g++ -std=c++11 -Wall -g -c test.cpp -o test.o

interp.o: interp.cpp interp.h r0.h
	g++ -std=c++11 -Wall -g -c interp.cpp -o interp.o

compile.o: compile.cpp compile.h r0.h
	g++ -std=c++11 -Wall -g -c compile.cpp -o compile.o

clean:
	rm -f *.o main

