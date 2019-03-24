CC = g++
FLAGS = -Wall -Wextra

fileinfo.o: fileinfo.hpp fileinfo.cpp
	${CC} ${FLAGS} -I. -c fileinfo.cpp

lib: fileinfo.o
	ar cr libfileinfo.a fileinfo.o

main.o: main.cpp
	${CC} ${FLAGS} -I. -c main.cpp

mimels: main.o lib
	${CC} ${FLAGS} main.o -L. -lfileinfo -lmagic -o mimels

clean:
	rm -rf *.o
	rm mimels
	rm *.a
