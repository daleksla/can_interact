# can_interact
## examples/README

Examples demonstrating usage of can_interact library.

### Native C Library

Navigate to `c/` directory

#### Structure of examples

##### can_writer

Takes 3 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to write (e.g. 0x100)
3. the data to write (e.g. 100)

Steps:
1. A call to `can_interact_init` is made to create a descriptor to the CAN socket
2. `can_interact_encode` is called to convert the cmd-line argument into a specific endianness for transport in a `can_frame` struct
3. `can_interact_send_frame` is used to then publish the can frame to the socket descriptor
4. `can_interact_fini` should be called the cleanup and release the socket and associated resources

##### can_reader

Takes 2 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to read (e.g. 0x100)

Steps:
1. A call to `can_interact_init` is made to create a descriptor to the CAN socket
2. A call to `can_interact_filter` sets up kernel level filtering
3. A `can_frame` struct is declared and `can_interact_get_frame` gets data and sets its values
4. `can_interact_decode` is called to use the incoming bytes and it's length and format them for the target device into an existing `double`, `long unsigned` or `long int` variable
5. Prints value fetched
6. `can_interact_fini` is called the cleanup and release the socket and associated resources

#### Building

Building requires uses GNU `make` utility - run `make examples`

### CXX API

Navigate to `cxx/` directory

#### Structure of examples

##### can_writer

Takes 3 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to write (e.g. 0x100)
3. the data to write (e.g. 100)

Steps:
1. Declare `can_interact::CAN` object, providing device name to constructor
2. Encode your data using `can_interact::encode` - this will generate a `can_frame` struct
3. Call the `frame` method of the `can_interact::CAN` object declared previously, providing the newly created `can_frame` object as its argument

##### can_reader

Takes 2 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to read (e.g. 0x100)

Steps:
1. Declare `can_interact::CAN` object, providing device name to constructor
2. Call the (kernel filtering) `filter` method of aforementioned `can_interact::CAN` object, providing a C-style array or the STL's `std::vector` / `std::array` containers containing desired IDs of incoming CAN frames
3. Call the `frame` (getter) method of the `can_interact::CAN` object to return the latest message in a `can_frame` struct
4. Call `can_interact::decode` function, providing `can_frame` struct as the argument

### Running examples

First, open a connection to a CAN device. It's easy to test using VCAN
> Install and use `can-utils` library
> Then run `sudo ip link add dev vcan0 type vcan && sudo ip link set up vcan0`

Then open 2 terminal windows, with each directed to the `examples/c` or `examples/cxx` directories
* In terminal 1, run `can_reader.o can0 0x100` - it'll stall till a value is provided
* In terminal 2, run `can_writer.o can0 0x100 100`

Upon running the command in the second terminal, the `can_reader.o` should print 100 and quit.

Experiment:
* Do it again, sending all sorts of different numbers
* Modify the source codes to change the endianness, recompile and do it again

Any issues, don't hesistate to open an issue. I'll fix it when I can or make a pull request and I'll check it
