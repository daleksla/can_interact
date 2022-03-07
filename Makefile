CC=gcc

all: build clean

build:
	@echo "Building library..."
	$(CC) -g -c can_reader.c -o can_reader_lib.o
	@echo "Building and linking executable..."
	$(CC) -g can_reader_lib.o source.c -o can_reader

clean:
	@echo "Deleting" *.o
	@rm *.o
