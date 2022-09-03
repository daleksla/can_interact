CC=gcc --std=c89 -Wextra -Wall -pedantic -Wconversion -g
CXX=g++ --std=c++11 -Wextra -Wall -pedantic -Wconversion -g

all: lib examples

lib:
	@echo "Building can_interact library..."
	$(CC) -c can_interact.c -lm -o can_interact.o

examples: lib
	@echo "Building and linking examples using can_interact library..."
	$(CC) -I ./ can_interact.o examples/c/can_reader.c -lm -o examples/c/can_reader.o
	$(CC) -I ./ can_interact.o examples/c/can_writer.c -lm -o examples/c/can_writer.o
	$(CXX) -I ./ can_interact.o examples/cxx/can_reader.cc -lm -o examples/cxx/can_reader.o
	$(CXX) -I ./ can_interact.o examples/cxx/can_writer.cc -lm -o examples/cxx/can_writer.o

clean:
	@echo "Deleting" *.o examples/*.o
	@rm *.o examples/*/*.o 2> /dev/null # in case there are no object files
