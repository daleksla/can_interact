# can_interact
## README

Functionality to easily read and write to CAN bus using Linux SocketCAN API in C.

### Building

Building requires use of GNU `make` utility. Number of commands available:
* `make`/`make all` - builds all files (library, examples)
* `make examples` - builds all examples (with library as a prerequisite)
* `make example_can_reader` - builds CAN reader example only
* `make example_can_writer` - builds CAN writer example only
* `make lib` - builds can_interact library only
* `make clean` - deletes all compiled output

Passing `CC` argument allows you specify the compiler information (e.g. `CC="gcc -std=c89"`, `CC="clang++ --std=c++2a"`).

Note: no name mangling is performed if compiled with a standard CXX compiler. This is done to allow for flexible linkage.

### Using

Simply include `can_interact.h` and link subsequent compiler output with `can_interact.o` object file.

See `examples` directory for how to use library.

***

Written in C89, with compatability for all C standards beyond this and all standard-following CXX compilers.

See LICENSE for terms of usage.
