CC=gcc --std=c89 -Wextra -Wall -pedantic -Wconversion -g

all: lib examples

lib:
	@echo "Building can_interact library..."
	$(CC) -c can_interact.c -o can_interact.o
	$(CC) -I ./ can_interact.o examples/tmp.c -o examples/tmp.o

examples: lib example_can_reader example_can_writer
	@echo "Building and linking examples using can_interact library..."

example_can_reader: lib
	$(CC) -I ./ can_interact.o examples/can_reader.c -o examples/can_reader.o

example_can_writer: lib
	$(CC) -I ./ can_interact.o examples/can_writer.c -o examples/can_writer.o

clean:
	@echo "Deleting" *.o examples/*.o
	@rm *.o examples/*.o 2> /dev/null # in case there are no object files
