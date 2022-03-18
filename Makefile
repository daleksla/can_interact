CC?=gcc

all: lib exes

lib:	
	@echo "Building can_interact library..."
	$(CC) -g -c can_interact.c -o can_interact.o

exes:
	@echo "Building and linking can_reader example..."
	$(CC) -g can_interact.o can_reader.c -o can_reader

clean:
	@echo "Deleting" *.o
	@rm *.o
