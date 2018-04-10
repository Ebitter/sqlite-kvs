INCLUDE_DIR=$(shell echo ~)/local/include
LIBRARY_DIR=$(shell echo ~)/local/lib
DESDTIR=/
PREFIX=/usr
GITREF=\"$(shell git log | head -n 1 | awk '{ print $$2; }')\"
GITSTATUS=\"$(shell echo 'if [ "x$$(git status -s)" == "x" ]; then echo "clean"; else echo "dirty"; fi' | bash)\"

CXX=g++
CXXFLAGS=-DGITREF=${GITREF} -DGITSTATUS=${GITSTATUS} -L${LIBRARY_DIR} -I${INCLUDE_DIR} -march=native -O3 -flto -std=c++14 -fPIC -Wall -Wextra -fopenmp -fno-omit-frame-pointer -g

all: test

install: all src/sqlite_kvs.h
	mkdir -p ${DESTDIR}/${PREFIX}/include/sqlite_kvs/
	cp src/sqlite_kvs.h ${DESTDIR}/${PREFIX}/include/sqlite_kvs/

test: src/test.cc src/sqlite_kvs.h
	${CXX} ${CXXFLAGS} -o test src/test.cc -lsqlite3

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
	rm -rf test
