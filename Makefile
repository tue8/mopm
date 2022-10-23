SRC=$(wildcard src/*.c)
ASRC=$(wildcard include/*.c include/*/*.c src/*/*.c)
CFLAGS=-I include -L lib -l curl -l jansson
BINS=mopm.exe ins.exe unins.exe

all: $(BINS)

debug: CFLAGS+=-g -O0
debug: $(BINS)

$(BINS): $(SRC)
	gcc $(ASRC) src/$(subst .exe,.c,$@) $(CFLAGS) -o bin/$@

.PHONY: all debug