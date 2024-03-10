CC=clang
CFLAGS=
LDFLAGS=

.PHONY: all header static folders clean

all: header static

folders:
	mkdir build

header:
	@echo "header not yet implemented"

static: src/cvkstart.c | folders
	$(CC) -c src/cvkstart.c $(CFLAGS) -o build/cvkstart.o
	ar rvs libcvkstart.a build/cvkstart.o

clean:
	rm -rf build
	rm libcvkstart.a
