#ifndef CAN_INTERACT_H
#define CAN_INTERACT_H
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __linux__
#include <linux/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>
#include <endian.h>
#else
#error No support for non-Linux OS
#endif /* __linux__ */

/**
 * @brief C-style Functionality declarations of library code to be used to usefully read and write to CAN bus
 * For implementation for the CXX API, see can_interact.hh
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum can_interact_endianness {
    /**
     * @brief enum can_interact_init - provided to easily dictate whether a value is big or little in terms of endian formatting
     */
    ENDIAN_BIG = 0,
    ENDIAN_LITTLE
};

enum can_interact_data_type {
    /**
     * @brief enum can_interact_data_type - provided to easily dictate whether a value provided is signed or unsigned or ieee 745 float
     */
    DATA_TYPE_UNSIGNED = 0, /* unsigned long int */
    DATA_TYPE_SIGNED, /* signed long int */
    DATA_TYPE_FLOAT /* ieee 754 double float */
};

/**
 * @brief can_interact_init - initialises CAN connection to specific network device via low level syscalls
 *
 * @param const char* - c-string (null terminated) to CAN device name
 * @param int* - pointer to varibale to initialise as socket descriptor
 *
 * @return int - error code
 * Note: 0 on success, for non-zero values refer to errno codes for other writing errors
 * Please see errno documentation for further elaboration on non-zero return codes
 */
int can_interact_init(int *socket, const char *device_name);

/**
 * @brief can_interact_filter - function applies kernel level filtering to socket to relevant CAN frames
 *
 * @param const uint32_t* - const array of (hex) ids to request from kernel filter
 *
 * @param const size_t - length of array of hex ids to filter for
 *
 * @param const int* - pointer to socket descriptor
 *
 * @return int - error code
 * Note: 0 on success, for non-zero values refer to errno codes for other writing errors
 * Please see errno documentation for further elaboration on non-zero return codes
 */
int can_interact_filter(const uint32_t *filter_ids, const size_t len_ids, const int *socket);

/**
 * @brief can_interact_get_frame - function gets can frame from stream associated to descriptor
 *
 * @param struct can_frame* - pointer to can_frame to write to
 *
 * @param const int* - socket descriptor
 *
 * @return int - error code
 * Note: 0 on success, for non-zero values refer to errno codes for other writing errors
 * Please see errno documentation for further elaboration on non-zero return codes
 */
int can_interact_get_frame(struct can_frame *frame, const int *socket);

/**
 * @brief can_interact_decode - converts array of length x containing bytes into value
 *
 * @param const struct can_frame* - const pointer to can_frame containing message
 *
 * @param const enum can_interact_data_type - datatype of provided value to encode
 * DATA_TYPE_SIGNED indicates signed, DATA_TYPE_UNSIGNED indicates unsigned, DATA_TYPE_FLOAT for IEEE754 float
 *
 * @param const enum can_interact_endianness - enum indicating the output byte order
 * ENDIAN_LITTLE is little, ENDIAN_BIG is big
 *
 * @param void* - destination of decoding
 * Should be a pointer to a uint64_t, int64_t or double, dependant on the conversion that will occur
 * (i.e. DATA_TYPE_UNSIGNED = uint64_t, DATA_TYPE_SIGNED = int64_t, DATA_TYPE_FLOAT = double)
 *
 * @return int - whether values have been decoded correctly
 * 0 exit code == success, 1 means number of bytes to decode are below 0 / higher than 8, 2 means bytes provided are not 4 / 8 bytes in length when dealing with floating point values
 */
int can_interact_decode(const struct can_frame *frame, const enum can_interact_data_type type, const enum can_interact_endianness endianness, void* dest);

/**
 * @brief can_interact_encode - serialises and packages number into LINUX can_frame structure
 *
 * @param const canid_t - ID of to-be-sent CAN frame
 *
 * @param const void* - pointer of value to translate
 *
 * @param const uint8_t - size of data coming in
 *
 * @param const enum can_interact_data_type - datatype of provided value to encode
 * DATA_TYPE_SIGNED indicates signed, DATA_TYPE_UNSIGNED indicates unsigned, DATA_TYPE_FLOAT for IEEE754 float / double
 *
 * @param const enum can_interact_endianness - enum indicating the output byte order
 * ENDIAN_LITTLE is little, ENDIAN_BIG is big
 *
 * @param struct can_frame* - pointer to existing LINUX can frame to be initialised
 *
 * @return int - exit code
 * 0 exit code == success, 1 means number of bytes to decode are invalid (note: should be 4/8 bytes if floating point, 1-8 bytes if (unsigned/signed) integer)
 */
int can_interact_encode(const canid_t id, const void *value, const uint8_t len, const enum can_interact_data_type type, const enum can_interact_endianness endianness, struct can_frame *frame);

/**
 * @brief can_interact_send_frame - function sends can frame to stream associated to provided descriptor
 *
 * @param const struct can_frame* - pointer to LINUX can frame to send
 *
 * @param const int* - socket descriptor to send can frame to
 *
 * @return int - error code
 * Note: 0 on success, for non-zero values refer to errno codes for other writing errors
 * Please see errno documentation for further elaboration on non-zero return codes
 */
int can_interact_send_frame(const struct can_frame *frame, const int *socket);

/**
 * @brief can_interact_fini - frees CAN connection
 *
 * @param const int* - pointer to varibale to initialise as socket descriptor
 *
 * @return int - error code
 * Note: 0 on success, for non-zero values refer to errno codes for other writing errors
 * Please see errno documentation for further elaboration on non-zero return codes
 */
int can_interact_fini(const int *socket);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_INTERACT_H */
