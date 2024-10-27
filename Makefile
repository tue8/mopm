SRC=$(wildcard src/*.c)
ASRC=$(wildcard include/*.c include/*/*.c src/*/*.c)
CFLAGS=-I include -L lib -l curl -l jansson -l archive.dll -static -static-libgcc
BINS=mopm mo-get mo-remove

all: $(BINS)

debug: CFLAGS+=-g -O0 -D_DEBUG
debug: $(BINS)

$(BINS): $(SRC)
	gcc $(ASRC) src/$@.c $(CFLAGS) -o bin/$@ 

.PHONY: all debug