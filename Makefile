fileinfo.o: fileinfo.hpp fileinfo.cpp
	g++ -I. -c fileinfo.cpp

lib: fileinfo.o
	ar cr libfileinfo.a fileinfo.o

main.o: main.cpp
	g++ -I. -c main.cpp

mimels: main.o lib
	g++ main.o -L. -lfileinfo -lmagic -o mimels

clean:
	rm -rf *.o
	rm mimels
	rm *.a
