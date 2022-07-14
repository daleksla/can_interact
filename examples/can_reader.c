#include <stdio.h> /* io */
#include <unistd.h> /* syscalls */
#include <stdlib.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <argp.h>

#include "can_interact.h" /* functionality containing both code to read and write from socket */

/**
 * @brief Example source code of reading from CAN using can_interact functionality
 * File would almost certainly need to be modified to read and output select IDs, etc..
 */

#pragma GCC diagnostic ignored "-Wmissing-field-initializers" /* Below is some argp stuff. I'm ignoring some of the 'errors' */
#pragma GCC diagnostic push

static char args_doc[] = "NET_DEVICE_NAME FRAME_ID"; /* description of non-option specified command line arguments */
static char doc[] = "can_reader -- reads frame from CAN device with a specific ID"; /* general program documentation */
const char* argp_program_bug_address = "salih.msa@outlook.com";
static struct argp_option options[] = {{0}};

struct arguments {
/**
 * @brief struct arguments - this structure is used to communicate with parse_opt (for it to store the values it parses within it)
 */
	char *args[2];  /* args for (string) params */
};

/**
 * @brief parse_opt - deals with given arguments based on given argumentsK
 * @param int - int correlating to char storing argument key
 * @param char* - argument string associated with argument key
 * @param struct argp_state* - pointer to argp_state struct storing information about the state of the option parsing
 * @return error_t - number storing 0 upon successfully parsed values, non-zero exit code otherwise
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = (struct arguments*)state->input;

	switch (key) {
	case ARGP_KEY_ARG: {
		if(state->arg_num >= 2) {
			argp_usage(state);
		}
		arguments->args[state->arg_num] = arg;
		break;
	}
	case ARGP_KEY_END: {
		if (state->arg_num < 2) {
			argp_usage(state);
		}
		break;
	}
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main(int argc, char **argv)
{
	/* Initialisation */
	struct arguments arguments; /* stores argp args */
	struct argp argp = { /* argp - The ARGP structure itself */
		options, /* options */
		parse_opt, /* callback function to process args */
		args_doc, /* names of parameters */
		doc /* documentation containing general program description */
	};
	int s; /* SOCKET */
	unsigned int filter_ids[1]; /* kernel filter ids */
	long int frame_id; struct can_frame frame;
	double val;
	
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	frame_id = strtol(arguments.args[1], NULL, 0);

	/* Main functionality */
	can_interact_init(&s, arguments.args[0]);
	filter_ids[0] = (unsigned)strtol(arguments.args[1], NULL, 0); /* gps accel */
	can_interact_filter(filter_ids, 1, &s);

	fprintf(stdout, "attempted read from frame 0x%x from can device %s\n", (unsigned int)frame_id, arguments.args[0]);

	while (1) {
		if (can_interact_get_frame(&frame, &s) != 0) {
			fprintf(stderr, "Error reading from CAN bus\n");
			abort();
		}

		if (frame.can_id == frame_id) {
			can_interact_decode(
				frame.data,
				frame.can_dlc,
				DATA_TYPE_SIGNED,
				ENDIAN_BIG,
				&val
			);
			fprintf(stdout, "val read from frame with id 0x%x: %f\n", (unsigned int)frame_id, val);
		} else {
			fprintf(stderr, "Warning: kernel filtering not setup\n");
		}
	}

	/* E(nd)O(f)P(rogram) */
	close(s);
	return 0;
}
