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
 * @brief Functionality declarations of library code to be used to usefully read and write to CAN bus
 * File in nearly all cases designed and works as independant from any reading specifications etc..
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
    DATA_TYPE_UNSIGNED = 0,
    DATA_TYPE_SIGNED,
    DATA_TYPE_FLOAT
};

/**
 * @brief can_interact_init - initialises CAN connection to specific network device via low level syscalls
 *
 * @param[in] const char* - c-string (null terminated) to CAN device name
 * @param[out] int* - pointer to varibale to initialise as socket descriptor
 *
 * @return int - error code
 * Note: 0 on success, 1 unable to open socket, 2 upon failure to bind to given device
 */
int can_interact_init(int *socket, const char *device_name);

/**
 * @brief can_interact_filter - function applies kernel level filtering to socket to relevant CAN frames
 * @param[in] const unsigned int* - const array of (hex) ids to request from kernel filter
 * @param[in] const size_t - length of array of hex ids to filter for
 * @param[in] const int* - pointer to socket descriptor
 * @return int - error code (0 on success, non-zero otherwise)
 */
int can_interact_filter(const unsigned int *filter_ids, const size_t len_ids, const int *socket);

/**
 * @brief can_interact_get_frame - function gets can frame from stream associated to descriptor
 * @param[out] struct can_frame* - pointer to can_frame to write to
 * @param[in] const int* - socket descriptor
 * @return int - 0 whether read was successful, 1 if no bytes were read (whether is be due to a bad socket or an unavailability of data is ambigiuous)
 */
int can_interact_get_frame(struct can_frame *frame, const int *socket);

/**
 * @brief can_interact_decode - converts array of length x containing hex bytes into a uint64
 * @param[in] const uint8_t* - const array of hex bytes
 * @param[in] const uint8_t - length of hex bytes array
 * @param[in] const enum can_interact_data_type - type of value
 * @param[in] const enum can_interact_endianness - endianess. LITTLE_ENDIAN_VAL is little, BIG_ENDIAN_VAL is big
 * @return[out] uint64_t - interpretted value as unsigned int from hex bytes, taking other params into account
 */
uint64_t can_interact_decode(const uint8_t *in_bytes, const uint8_t len_bytes, const enum can_interact_data_type type, const enum can_interact_endianness endianness);

/**
 * @brief can_interact_encode - converts a uint64 number to an array of hex bytes
 * @param[im] const uint64_t - value to translate
 * @param[out] uint8_t* - array destination of bytes (space needed = 8 bytes)
 * @param[in] const enum can_interact_data_type - whether the bytes are storing a signed value or not. SIGNED_VAL indicates signed, UNSIGNED_VAL indicates unsigned
 * @param[in] const enum can_interact_endianness - endianess. LITTLE_ENDIAN_VAL is little, BIG_ENDIAN_VAL is big
 * @return uint8_t - number of destination array elements actually used
 */
uint8_t can_interact_encode(const uint64_t value, uint8_t* byte_space, const enum can_interact_data_type type, const enum can_interact_endianness endianness);

/**
 * @brief can_interact_make_frame - initialises can frame using provided data to fill out properties
 * @param[in] const canid_t - desired can id for the provided data
 * @param[in] uint8_t* - array of bytes to send
 * @param[in] const uint8_t - length of data
 * @param[out] struct can_frame* - pointer to can frame to write values to
 * @return int - error code (0 on success, non-zero on failure)
 */
int can_interact_make_frame(const canid_t frame_id, const uint8_t *bytes, const uint8_t len_bytes, struct can_frame *frame);

/**
 * @brief can_interact_send_frame - function sends can frame to stream associated to provided descriptor
 * @param[in] const struct can_frame* - pointer to can frame to send
 * @param[in] const int* - socket descriptor to send can frame to
 * @return int - error code (0 on success, non-zero on failure)
 */
int can_interact_send_frame(const struct can_frame *frame, const int *socket);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_INTERACT_H */
