CCFLAGS = -std=c99 -pedantic -Wunused -Werror

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CCFLAGS +=  -framework CoreFoundation -framework CoreServices
endif

SOURCES=src/birder.c

all: CCFLAGS += -O3
all: bin/birder

debug: CCFLAGS += -g3
debug: bin/birder

bin/birder: $(SOURCES)
	mkdir -p bin
	$(CC) $(CCFLAGS) $(SOURCES) -o bin/birder

clean:
	rm -rf bin*

install: bin/birder
	cp bin/birder /usr/local/bin/

.BOGUS:
