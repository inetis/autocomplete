GCCV=47
CC=gcc${GCCV}
CFLAGS=-std=c99 -O -I./mongoose
LDFLAGS=-L./ -lpthread -lstdc++
CXX=g++${GCCV}
CXXFLAGS=-std=c++0x -O -I./src

TARGETS=server testrun

.PATH: src demo mongoose

all:${TARGETS}

server: mongoose.o server.o libac.a
	${CC} ${.ALLSRC} -o ${.TARGET} ${LDFLAGS}

testrun: testrun.o Autocomplete.o AutocompleteUtils.o
	${CXX} ${.ALLSRC} -o ${.TARGET}

mongoose.o: mongoose/mongoose.c

mongoose/mongoose.c:
	fetch -o- "http://mongoose.googlecode.com/files/mongoose-3.3.tgz" | tar -xf-

libac.a: Autocomplete.o AutocompleteUtils.o ac.o
	ar rcs libac.a Autocomplete.o AutocompleteUtils.o ac.o

clean:
	rm -rf a.out *.o *.so *.a
	rm -rf server testrun
	rm -rf mongoose/ mongoose-*.tgz

