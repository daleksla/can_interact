#ifndef CAN_INTERACT_H
#define CAN_INTERACT_H
#pragma once

#include <stddef.h>
#include <stdint.h>

/**
  * @brief Functionality declarations of library code to be used to usefully read and write to CAN bus
  * File in nearly all cases designed and works as independant from any reading specifications etc..
  */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum EndianType {
    BIG_ENDIAN_VAL = 0,
    LITTLE_ENDIAN_VAL
} ;

/**
 * @brief can_socket_init - initialises CAN connection to specific network device via low level syscalls
 * @param const char* - c-string to CAN device name
 * @return int - non-zero positive integer socket descriptor, -1 unable to open socket, -2 upon failure to bind to given device
 */
int can_socket_init(const char*) ;

/**
 * @brief apply_can_fitler - function applies kernel level filtering to socket to relevant CAN frames
 * @param const unsigned int* - const array of hex ids to request from kernel filter
 * @param const size_t - length of array of hex ids to filter for
 * @param const int - socket descriptor
 */
void apply_can_fitler(const unsigned int*, const size_t, const int) ;

/**
 * @brief get_can_frame - function gets can frame from stream associated to descriptor
 * @param struct can_frame* - pointer to can_frame to write to
 * @param const int - socket descriptor
 * @return int - 0 whether read was successful, 1 if no bytes were read (whether is be due to a bad socket or an unavailability of data is ambigiuous)
 */
int get_can_frame(struct can_frame*, const int) ;

/**
 * @brief hex_bytes_to_number - converts array of length x containing hex bytes into a uint64
 * @param const uint8_t* - const array of hex bytes
 * @param const size_t - length of hex bytes array (undefined behaviour is exceeding 8)
 * @param const enum EndianType - endianess. LITTLE_ENDIAN_VAL is little, BIG_ENDIAN_VAL is big
 * @return uint64_t - interpretted value as unsigned int from hex bytes, taking other params into account
 */
uint64_t hex_bytes_to_number(const uint8_t*, const size_t, const enum EndianType) ;

/**
 * @brief number_to_hex_bytes - converts a uint64 number to an array of hex bytes
 * @param const uint64_t - interpretted value as unsigned int from hex bytes, taking other params into account
 * @param uint8_t* - array destination of hex bytes (space needed = 8 bytes)
 * @param const enum EndianType - endianess. LITTLE_ENDIAN_VAL is little, BIG_ENDIAN_VAL is big
 * @return size_t - number of destination array elements actually used
 */
size_t number_to_hex_bytes(const uint64_t, uint8_t*, const enum EndianType) ;

/**
 * @brief create_can_frame - initialises can frame using provided data to fill out properties
 * @param const canid_t - desired can id for the provided data
 * @param const size_t - length of data undefined behaviour < 0 || > 8)
 * @param struct can_frame* - pointer to can frame to write values to
 */
void create_can_frame(const canid_t, const uint8_t*, const size_t, struct can_frame*) ;

/**
 * @brief send_can_frame - function sends can frame to stream associated to provided descriptor
 * @param const struct can_frame* - pointer to can frame to send
 * @param const int - socket descriptor to send can frame to
 */
int send_can_frame(const struct can_frame*, const int) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_INTERACT_H */
