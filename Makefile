CC=gcc --std=c89 -Wextra -Wall -pedantic -Wconversion -g

all: lib examples

lib:
	@echo "Building can_interact library..."
	$(CC) -c can_interact.c -lm -o can_interact.o

examples: lib example_can_reader example_can_writer
	@echo "Building and linking examples using can_interact library..."

example_can_reader: lib
	gcc --std=c11 -I ./ can_interact.o examples/can_reader.c -lm -o examples/can_reader.o

example_can_writer: lib
	gcc --std=c11 -I ./ can_interact.o examples/can_writer.c -lm -o examples/can_writer.o

clean:
	@echo "Deleting" *.o examples/*.o
	@rm *.o examples/*.o 2> /dev/null # in case there are no object files
