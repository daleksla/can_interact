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
1. A call to `can_interact_init` is made to create a descriptor to the CAN socket
2. `can_interact_encode` is called to convert the cmd-line argument into a specific endianness for transport
3. A `can_frame` struct is declared and `can_interact_make_frame` initialises it
4. `can_interact_send_frame` is used to then publish the can frame to the socket descriptor

#### can_reader

Takes 2 command line arguments:
1. the device name (e.g. can0)
2. the hex id of the frame to read (e.g. 0x100)

Steps:
1. A call to `can_interact_init` is made to create a descriptor to the CAN socket
2. A call to `can_interact_filter` sets up kernel level filtering
3. A `can_frame` struct is declared and `can_interact_get_frame` gets data and sets its values
4. `can_interact_decode` is called to get the incoming bytes and format them for the target device
5. Prints value fetched

### Building

Building requires use of GNU `make` utility - run `make examples`.

### Running

First, open a connection to a CAN device
> I opened mine using `sudo --stdin ip link set can0 up type can bitrate 1000000`
* VCAN would work fine.
> Install and use `can-utils` library, it's honestly great

Then open 2 terminal windows.
* In terminal 1, run `can_reader.o can0 0x100` - it'll stall till a value is provided
* In terminal 2, run `can_writer.o can0 0x100 100`

Upon running the command in the second terminal, the `can_reader.o` should print 100 and quit.

Experiment:
* Do it again, sending all sorts of different numbers
* Modify the source codes to change the endianness, recompile and do it again

Any issues, don't hesistate to open an issue. I'll fix it when I can or make a pull request and I'll check it
