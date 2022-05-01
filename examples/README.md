# can_interact
## examples/README

Examples demonstrating usage of can_interact library.

###

### Structure of examples

#### can_writer

Takes 3 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to write (e.g. 0x100)
3. the data to write (e.g. 100)

Steps:
1. A call to `can_socket_init(<arg0>)` is made to create a descriptor to the CAN socket
2. `number_to_hex_bytes(<arg3 as num>, <uint8_t array of len 8>, BIG_ENDIAN_VAL)` is called
  * Note: arg 3 is an enum stating the endian ordering desired for the array of bytes storing the provided number - in this case we want it as a big endian value
  * It returns a number regarding the number of bytes used to store the hex values. This can be ignored
3. A `can_frame` struct is declared and `create_can_frame(<arg1 as num>, <uint8_t array of len 8>, <number of bytes from above or 8>, <pointer to can_frame struct>)` initialises it
4. `send_can_frame(<pointer to can_frame variable>, <socket des>)` is used to then publish the can frame to the socket descriptor

#### can_reader

Takes 2 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to read (e.g. 0x100)

Steps:
1. A call to `can_socket_init(arg0)` is made to create a descriptor to the CAN socket
2. A call to `apply_can_fitler(<array of hex ids, one being arg1>, <socket>)` sets up kernel level filtering
3. A `can_frame` struct is declared and `get_can_frame(<pointer to can_frame variable>, <socket>)` gets data and sets it
4. `hex_bytes_to_number(<uint8_t array of len 8>, <data len>, BIG_ENDIAN_VAL)` is called
  * Note: arg 3 is an enum stating the endian ordering desired for the array of bytes storing the provided number - in this case we want it as a big endian value
  * It returns a number which is the assembled bytes
5. Prints value fetched

### Building

Building requires use of GNU `make` utility - run `make examples`.

### Running

First, open a connection to a CAN device. I opened mine using `sudo --stdin ip link set can0 up type can bitrate 1000000`

Then open 2 terminal windows.
* In terminal 1, run `can_reader.o can0 0x100` - it'll stall till a value is provided
* In terminal 2, run `can_writer.o can0 0x100 100`

Upon running the command in the second terminal, the `can_reader.o` should print 100 and quit.
