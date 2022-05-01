CC?=gcc

all: lib examples

lib:
	@echo "Building can_interact library..."
	$(CC) -g -c can_interact.c -o can_interact.o

examples: lib example_can_reader example_can_writer
	@echo "Building and linking examples using can_interact library..."

example_can_reader: lib
	$(CC) -g -I . can_interact.o examples/can_reader.c -o examples/can_reader.o

example_can_writer: lib
	$(CC) -g -I . can_interact.o examples/can_writer.c -o examples/can_writer.o

clean:
	@echo "Deleting" *.o
	@rm *.o examples/*.o 2> /dev/null # in case there are no object files
