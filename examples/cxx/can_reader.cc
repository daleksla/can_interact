#include <string>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <argp.h>

#include "can_interact.hh" // functionality containing both code to read and write from socket 

/**
 * @brief Example source code of reading from CAN using CXX API for can_interact functionality
 * File would almost certainly need to be modified to read and output select IDs, etc..
 */

#pragma GCC diagnostic ignored "-Wmissing-field-initializers" /* Below is some argp stuff. I'm ignoring some of the 'errors' */
#pragma GCC diagnostic push

static char args_doc[] = "NET_DEVICE_NAME FRAME_ID" ; // description of non-option specified command line arguments
static char doc[] = "can_reader -- reads frame from CAN device with a specific ID" ; // general program documentation
const char* argp_program_bug_address = "salih.msa@outlook.com" ;
static struct argp_option options[] = {{0}} ;

struct arguments {
/**
  * @brief struct arguments - this structure is used to communicate with parse_opt (for it to store the values it parses within it)
  */
	std::string args[2] ; // args for (string) params
} ;

/**
  * @brief parse_opt - deals with given arguments based on given argumentsK
  * @param int - int correlating to char storing argument key
  * @param char* - argument string associated with argument key
  * @param struct argp_state* - pointer to argp_state struct storing information about the state of the option parsing
  * @return error_t - number storing 0 upon successfully parsed values, non-zero exit code otherwise
  */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = (struct arguments*)state->input ;

	switch (key) {
	case ARGP_KEY_ARG:
	{
		if(state->arg_num >= 2)
		{
			argp_usage(state) ;
		}
		arguments->args[state->arg_num] = std::string(arg) ;
		break ;
	}
	case ARGP_KEY_END:
	{
		if (state->arg_num < 2)
		{
			argp_usage(state) ;
		}
		break ;
	}
	default:
		return ARGP_ERR_UNKNOWN ;
	}
	return 0;
}

#pragma GCC diagnostic pop /* end of argp, so end of repressing weird messages */

int main(int argc, char **argv)
{
	/* Initialisation */
	struct arguments arguments ; // stores argp args
	struct argp argp = { // argp - The ARGP structure itself
		options, // options
		parse_opt, // callback function to process args
		args_doc, // names of parameters
		doc // documentation containing general program description
	} ;
	argp_parse(&argp, argc, argv, 0, 0, &arguments) ;
	const std::array<unsigned int, 1> filter_ids = {
		static_cast<unsigned int>(
			std::stoul(arguments.args[1], nullptr, 16)
		) // kernel filter ids
	} ;

	/* Main functionality */
	can_interact::CAN can(arguments.args[0]) ; // initialise CAN communicator object
	can.filter(filter_ids) ; // set filter

	std::fprintf(stdout, "attempted read from frame 0x%x from can device %s\n", static_cast<unsigned>(filter_ids[0]), arguments.args[0].c_str()) ;

	can_frame frame ;
	try {
		frame = can.frame() ;
		if(frame.can_id == filter_ids[0])
		{
			const double val = can_interact::decode<double>(frame, ENDIAN_LITTLE) ;
			std::fprintf(stdout, "val read from frame with id 0x%x: %f\n", filter_ids[0], val) ;
		}
		else {
			std::fprintf(stderr, "Warning: kernel filtering not setup\n") ;
		}
	}
	catch(const std::exception& e) // can.frame() will throw exception in terms of errors
	{
		std::fprintf(stderr, "Error reading from CAN bus\n") ;
		std::abort() ;
	}

	/* E(nd)O(f)P(rogram) */
	// fd + socket cleanup is done via destructor
	return 0;
}
