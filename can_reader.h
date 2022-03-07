#ifndef CAN_READER_H
#define CAN_READER_H
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
  * @brief Library header of can_reader system
  * File in nearly all cases designed and works as independant from any reading specifications etc..
  */

/**
  * @brief can_socket_init - initialises CAN connection to specific network device via low level syscalls
  * @param const char* - c-string to CAN device name
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
  * @brief hex_bytes_to_number - converts array of length x containing hex bytes into a float
  * @param const uint8_t* - const array of hex bytes
  * @param const size_t - length of hex bytes array
  * @param const bool - endianess. 0 is little, 1 is big
  * @param const bool - signedness. 0 is unsigned. 1 is signed and requires interpretation of bytes as such
  * @return float - interpretted float from hex bytes, taking other params into account
  */
float hex_bytes_to_number(const uint8_t*, const size_t, const bool, const bool) ;

#endif // CAN_READER_H
