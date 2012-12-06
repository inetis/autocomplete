GCCV=47
CC=gcc${GCCV}
CFLAGS=-std=c99 -O -I./mongoose
LDFLAGS=-L./ -lpthread -lstdc++
CXX=g++${GCCV}
CXXFLAGS=-std=c++0x -O -I./src

.PATH: src demo mongoose

TARGETS=server testrun

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

quicktest: testrun cities.txt.small
	./testrun cities.txt.small

cities.txt.small: cities.txt
	random 100 < cities.txt > cities.txt.small

cities.txt:
	fetch -o- "http://www.maxmind.com/download/worldcities/worldcitiespop.txt.gz" | \
		gunzip | \
		iconv -f latin1 -t utf-8 | \
		cut -d, -f2,5 | \
		sed -E -e 's/,$/,1/' -e 's/[^a-z0-9 ,-]//g' -e 's/([^,]+),([0-9]+)/\2 \1/' \
		> ${.TARGET}

github:
	git commit -a
	git push origin master

clean:
	rm -rf a.out *.o *.so *.a
	rm -rf server testrun
	rm -rf mongoose/ mongoose-*.tgz
	rm -rf cities.txt.small

