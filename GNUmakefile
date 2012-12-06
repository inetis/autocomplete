GCCV=
CC=gcc${GCCV}
CFLAGS=-std=c99 -O -I./mongoose
LDFLAGS=-L./ -lpthread -lstdc++ -ldl
CXX=g++${GCCV}
CXXFLAGS=-std=c++0x -O -I./src

VPATH=src:demo:mongoose

TARGETS=server testrun

all:${TARGETS}

server: mongoose.o server.o libac.a
	${CC} mongoose.o server.o libac.a -o server ${LDFLAGS}

testrun: testrun.o Autocomplete.o AutocompleteUtils.o
	${CXX} $? -o $@

mongoose.o: mongoose/mongoose.c
	${CC} -c mongoose/mongoose.c ${CFLAGS}

mongoose/mongoose.c:
	wget -O- "http://mongoose.googlecode.com/files/mongoose-3.3.tgz" | tar -xzf-

libac.a: Autocomplete.o AutocompleteUtils.o ac.o
	ar rcs libac.a Autocomplete.o AutocompleteUtils.o ac.o

quicktest: testrun cities.txt
	./testrun cities.txt.small

cities.txt.small: cities.txt
	sort -R cities.txt | head -n10000 > cities.txt.small

cities.txt:
	wget -O- "http://www.maxmind.com/download/worldcities/worldcitiespop.txt.gz" | \
		gunzip | \
		iconv -f latin1 -t utf-8 | \
		cut -d, -f2,5 | \
		sed -E -e 's/,$/,1/' -e 's/[^a-z0-9 ,-]//g' -e 's/([^,]+),([0-9]+)/\2 \1/' \
		> $@

.PHONY: clean
clean:
	rm -rf a.out *.o *.so *.a
	rm -rf server testrun
	rm -rf mongoose/ mongoose-*.tgz
	rm -rf cities.txt.small

