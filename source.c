#include <stdio.h> // io
#include <unistd.h> // syscalls

#include <linux/can.h>
#include <linux/can/raw.h>

#include <argp.h>

#include "can_reader.h"

/**
  * @brief Source file of can_reader system
  * File would almost certainly need to be modified to read and output select IDs, etc..
  */

#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // Below is some argp stuff. I'm ignoring some of the 'errors'
#pragma GCC diagnostic push

    static char args_doc[] = "NET_DEVICE_NAME" ; // description of non-option specified command line arguments
    static char doc[] = "can_reader -- reads FSAI device messages from specified CAN device" ; // general program documentation
    const char* argp_program_bug_address = "salih.msa@outlook.com" ;
    static struct argp_option options[] = {
        {0}
    } ;
    struct arguments {
        /**
          * @brief struct arguments - this structure is used to communicate with parse_opt (for it to store the values it parses within it)
          */
        char* args[1] ;  /* args for params */
    } ;

    /**
      * @brief parse_opt - deals with given arguments based on given argumentsK
      * @param int - int correlating to char storing argument key
      * @param char* - argument string associated with argument key
      * @param struct argp_state* - pointer to argp_state struct storing information about the state of the option parsing
      * @return error_t - number storing 0 upon successfully parsed values, non-zero exit code otherwise
      */
    static error_t parse_opt(int key, char *arg, struct argp_state* state)
    {
        struct arguments* arguments = (struct arguments*)state->input;

        switch (key)
        {
            case ARGP_KEY_ARG:
                if(state->arg_num >= 1)
                {
                    argp_usage(state);
                }
                arguments->args[state->arg_num] = arg;
                break;
            case ARGP_KEY_END:
                if (state->arg_num < 1)
                {
                    argp_usage(state);
                }
                break;
            default:
                return ARGP_ERR_UNKNOWN;
        }
        return 0 ;
    }

#pragma GCC diagnostic pop // end of argp, so end of repressing weird messages

int main(int argc, char** argv)
{
    /* Initialisation */
    struct arguments arguments;
    static struct argp argp = { // argp - The ARGP structure itself
        options, // options
        parse_opt, // callback function to process args
        args_doc, // names of parameters
        doc // documentation containing general program description
    } ;
    argp_parse(&argp, argc, argv, 0, 0, &arguments); // override default arguments if provided

    const int s = can_socket_init(arguments.args[0]) ;
    unsigned int filter_ids[5] = {
        0x100, // front_left_dist
        0x101, // front_right*
        0x102, // rear_left*
        0x103, // rear_right*
        0x680, // gps
    } ;
    apply_can_fitler(filter_ids, 5, s) ;

    /* Main functionality */
    while(1)
    {
        struct can_frame frame ;
        int nbytes = read(s, &frame, sizeof(struct can_frame)) ;

        if(nbytes < 0)
        {
            fprintf(stderr, "Error reading from CAN\n") ;
            return 3 ;
        }

        switch(frame.can_id)
        {
            case 0x680: // GPS
            {
                fprintf(stdout, "GPS DEVICE MSG:\n") ;
                fprintf(stdout, "\tLat as single value: %f\n", (hex_bytes_to_number(frame.data, 4, false, true) / 10000) * 0.0166667) ;
                fprintf(stdout, "\tLon as single value: %f\n", (hex_bytes_to_number(frame.data+4, 4, false, true) / 10000) * 0.0166667) ;
            }
        }
        fprintf(stdout, "\n") ;
    }
//        for(size_t i = 0 ; i < frame.can_dlc ; ++i)

    /* E(nd)O(f)P(rogram) */
    close(s) ;
    return 0 ;
}
