# can_interact
## README

Functionality to easily read and write to CAN bus in C/C++ for GNU/Linux OSs.

### Building

Building requires use of GNU `make` utility. Number of commands available:
* `make`/`make all` - builds all files (library, examples)
* `make examples` - builds all examples (with library as a prerequisite)
* `make lib` - builds can_interact library only
* `make clean` - deletes all compiled output

C functionality written in C89.

C++ API is a thin-header only wrapper and is CXX11 compatible - no compilation needed.

### Using

Simply include `can_interact.h` and link subsequent compiler output with `can_interact.o` object file for C/C++.

If you are using C++, you may instead opt to use `can_interact.hh`, which acts as the C++ wrapper. This includes:
* Functions as-is from `can_interact.h`
* Namespaced functions (`can_interact::`) which return data directly & throw detailed exceptions in cases of errors
* (Where applicable,) support for `std::vector` and `std::array` arguments as-well C-style lists
* A `CAN` class (namespaced too) to establish & maintain connections and to receive & send messages over CAN

Note you will still need to link with `can_interact.o` as this is a thin wrapper with minimal functionality of their own.

See `examples` directory for how to use library.

***

See LICENSE for terms of usage.
