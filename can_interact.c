#define _DEFAULT_SOURCE

#include <unistd.h> /* syscalls */
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <endian.h>
#include <math.h>

#include <errno.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>

#include "can_interact.h"

#define BYTE_MAX_LENGTH sizeof(uint64_t) / sizeof(uint8_t)

/**
 * @brief C-style Functionality definitions of library code to be used to usefully read and write to CAN bus
 * For definitions for the CXX API, see can_interact.cc
 */

extern int errno ;

int can_interact_init(int *s, const char *net_device)
{
	struct ifreq ifr; /* used to configure net device */
	struct sockaddr_can addr; /* assigns address connections */

	*s = socket(PF_CAN, SOCK_RAW, CAN_RAW); /* creates communication endpoint to a given data source. request for raw network protocol access */
	if(*s == -1) {
		return (int)errno;
	}
	strcpy(ifr.ifr_name, net_device); /* copy name */
	ioctl(*s, SIOCGIFINDEX, &ifr); /*  manipulates the underlying device parameters of special files */
	memset(&addr, '\0', sizeof(addr)); /* clear existing struct */
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if(bind(*s, (struct sockaddr*)&addr, sizeof(addr)) == -1) { /* bind the socket to the CAN Interface */
		return (int)errno;
	}

	return 0;
}

int can_interact_filter(const uint32_t *filter_ids, const size_t filter_id_len, const int *socket)
{
	struct can_filter *filters;
	size_t i;
	int res;

	filters = (struct can_filter*)malloc(sizeof(struct can_filter) * filter_id_len);
	
	if (filters == NULL) {
		return (int)errno;
	}
	
	for(i = 0; i < filter_id_len; ++i) {
		filters[i].can_id = filter_ids[i];
		filters[i].can_mask = 0x1FFFFFFF; /* every bit must match filter (see https://www.cnblogs.com/shangdawei/p/4716860.html) */
	}
	
	res = setsockopt(*socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filters, sizeof(filters));
	free(filters);
	return res == 0 ? 0 : (int)errno ;
}

int can_interact_get_frame(struct can_frame* can_frame, const int *socket)
{
	const ssize_t nbytes = read(*socket, can_frame, sizeof(struct can_frame));
	return nbytes <= 0 ? (int)errno : 0; /* 0 = success, 1 = no data reading from CAN */
}

/**
 * @brief _p_can_interact_decode_float - INTERNAL METHOD. purely converts array of length x containing bytes into double type
 * @param const uint8_t* - const array of bytes
 * @param const uint8_t - length of array
 * @param const enum can_interact_endianness - endianess (see can_interact_endianness definition)
 * @return double - decoded double
 */
static double _p_can_interact_decode_float(const uint8_t *payload, const uint8_t data_len, const enum can_interact_endianness byte_order)
{
	double res; /* to store meaningful value of incoming float */
	uint64_t tmp; /* to store literal bytes */

	/* first, cast meaningful value into double var */
	if (data_len == 4) {
		res = *((float*)payload);
	} else {
		res = *((double*)payload);
	}

	/* now, we're going to:
	 * 1. bit cast every byte into an unsigned int of same length (as opposed to meaning cast)
	 * 2. convert endianness
	 * 3. bit cast back into the double
	 */
	memcpy(&tmp, &res, 8);
	if (byte_order == ENDIAN_LITTLE) {
		tmp = le64toh(tmp);
	} else if (byte_order == ENDIAN_BIG) {
		tmp = be64toh(tmp);
	}
	memcpy(&res, &tmp, 8);

	return res;
}

/**
 * @brief _p_can_interact_decode_int - INTERNAL METHOD. converts array of length x containing bytes into signed int len 64 btis
 * @param const uint8_t* - const array of bytes
 * @param const uint8_t - length of array
 * @param const enum can_interact_endianness - endianess (see can_interact_endianness definition)
 * @return int64_t - decoded signed 64-bit integer
 */
static int64_t _p_can_interact_decode_int(const uint8_t *payload, const uint8_t data_len, const enum can_interact_endianness byte_order)
{
	uint64_t result; /* [0,0,0,0,0,0,0,0] */
	uint8_t* blocks; /* array of 8 */
	result = 0;
	blocks = (uint8_t*)(&result);

	if (byte_order == ENDIAN_LITTLE) {
		memcpy(blocks, payload, data_len); /* copy data to start of number */
		result = le64toh(result);
	} else if (byte_order == ENDIAN_BIG) {
		memcpy(blocks + (BYTE_MAX_LENGTH - data_len), payload, data_len); /* copy number from end - data_len */
		result = be64toh(result);
	}

	return (int64_t)result;
}

/**
 * @brief _p_can_interact_decode_uint - INTERNAL METHOD. converts array of length x containing bytes into unsigned int len 64 btis
 * @param const uint8_t* - const array of bytes
 * @param const uint8_t - length of array
 * @param const enum can_interact_endianness - endianess (see can_interact_endianness definition)
 * @return uint64_t - decoded unsigned signed 64-bit integer (no explicit sign bitage)
 */
static uint64_t _p_can_interact_decode_uint(const uint8_t *payload, const uint8_t data_len, const enum can_interact_endianness byte_order)
{
	uint64_t result; /* [0,0,0,0,0,0,0,0] */
	uint8_t* blocks; /* array of 8 */
	result = 0;
	blocks = (uint8_t*)(&result);

	if (byte_order == ENDIAN_LITTLE) {
		memcpy(blocks, payload, data_len); /* copy data to start of number */
		if (((const int8_t*)payload)[data_len - 1] < 0) { /* if data is signed and we have negative value in significant bit */
			memset(blocks + data_len, 0xFF, BYTE_MAX_LENGTH - data_len);
		}
		result = le64toh(result);
	} else if (byte_order == ENDIAN_BIG) {
		memcpy(blocks + (BYTE_MAX_LENGTH - data_len), payload, data_len); /* copy number from end - data_len */
		if (((const int8_t*)payload)[0] < 0) { /* if data is signed and we have negative value in significant bit */
			memset(blocks, 0xFF, BYTE_MAX_LENGTH - data_len);
		}
		result = be64toh(result);
	}

	return result;
}

int can_interact_decode(const struct can_frame* frame, const enum can_interact_data_type type, const enum can_interact_endianness byte_order, void *dest)
{
	if (frame->can_dlc == 0 && frame->can_dlc > 8) { /* not valid for any kind of formatting */
		return 1;
	}

	if (type == DATA_TYPE_FLOAT) {
		if (frame->can_dlc != 4 && frame->can_dlc != 8) { /* not valid for float formatting */
			return 2;
		}
		*((double*)dest) = _p_can_interact_decode_float(frame->data, frame->can_dlc, byte_order);
	} else if (type == DATA_TYPE_SIGNED) {
		*((int64_t*)dest) = (int64_t)_p_can_interact_decode_int(frame->data, frame->can_dlc, byte_order);
	} else { /* uint */
		*((uint64_t*)dest) = _p_can_interact_decode_uint(frame->data, frame->can_dlc, byte_order);
	}

	return 0;
}

/**
 * @brief _p_can_interact_serialise_float - INTERNAL METHOD. Serialises single and double precision floating point values
 *
 * @param const void* - generic pointer to value to either type of float
 *
 * @param const uint8_t - unsigned char storing length of float in bytes (where 4 == float, 8 == double)
 *
 * @param const enum can_interact_endianness - endianness (order) of resulting bytes
 *
 * @param uint8_t* - pointer to byte string to dump serialised bytes into
 *
 * @return uint8_t - length / number of resultant bytes used
 * 1-8 == success, 0 == failure to encode due to incoming value's length issues (ie not 4 or 8 bytes)
 */
static uint8_t _p_can_interact_serialise_float(const void* host_number, const uint8_t len, const enum can_interact_endianness byte_order, uint8_t* dest_array)
{
	if (len != 4 && len != 8) {
		return 0;
	}

	/* deal with endianness here by doing a cast and overflow the value */
	if (byte_order == ENDIAN_LITTLE) {
		 if (len == 4) {
			*(uint32_t*)dest_array = htole32(*((uint32_t*)host_number));
		} else if (len == 8) {
			*(uint64_t*)dest_array = htole64(*((uint64_t*)host_number));
		}
	} else if(byte_order == ENDIAN_BIG) {
		if (len == 4) {
			*(uint32_t*)dest_array = htobe32(*((uint32_t*)host_number));
		} else if (len == 8) {
			*(uint64_t*)dest_array = htobe64(*((uint64_t*)host_number));
		}
	}

	return len;
}

/**
 * @brief _p_can_interact_serialise_ints - INTERNAL METHOD. Serialises signed and unsigned integer values
 *
 * @param const void* - generic pointer to value to either type of float
 *
 * @param const uint8_t - unsigned char storing length of float in bytes (where 4 == float, 8 == double)
 *
 * @param const enum can_interact_endianness - endianness (order) of resulting bytes
 *
 * @param uint8_t* - pointer to byte string to dump serialised bytes into
 *
 * @return uint8_t - length / number of resultant bytes used.
 * 1-8 == success, 0 == failure to encode due to incoming value's length issues (ie 0 or > 8 bytes)
 */
static uint8_t _p_can_interact_serialise_ints(const void* host_number, const uint8_t len, const enum can_interact_endianness byte_order, uint8_t* dest_array)
{
	if (len == 0 || len > 8) {
		return 0;
	}

	/* deal with endianness here by doing a cast and overflow the value */
	if (byte_order == ENDIAN_LITTLE) {
		if (len == 2) {
			*(uint16_t*)dest_array = htole16(*((uint16_t*)host_number));
		} else if (len == 4) {
			*(uint32_t*)dest_array = htole32(*((uint32_t*)host_number));
		} else if (len == 8) {
			*(uint64_t*)dest_array = htole64(*((uint64_t*)host_number));
		}
	} else if(byte_order == ENDIAN_BIG) {
		if (len == 2) {
			*(uint16_t*)dest_array = htobe16(*((uint16_t*)host_number));
		} else if (len == 4) {
			*(uint32_t*)dest_array = htobe32(*((uint32_t*)host_number));
		} else if (len == 8) {
			*(uint64_t*)dest_array = htobe64(*((uint64_t*)host_number));
		}
	}
	/* note: nothing is done if the number is one byte for obvious reasons */

	return len;
}

/**
 * @brief _p_can_interact_serialise - INTERNAL METHOD. Serialises incoming values
 *
 * @param const void* - generic pointer to value to either type of float
 *
 * @param const uint8_t - unsigned char storing length of float in bytes (where 4 == float, 8 == double)
 *
 * @param const enum can_interact_endianness - endianness (order) of resulting bytes
 *
 * @param uint8_t* - pointer to byte string to dump serialised bytes into
 *
 * @return uint8_t - length / number of resultant bytes used.
 * 1-8 == success, 0 == failure to encode due to incoming value's length issues (if input was float, then due to not 4 or 8 bytes. if input was integralm due to being 0 or >8 bytes)
 */
static uint8_t _p_can_interact_serialise(const void* host_number, const uint8_t len, const enum can_interact_data_type data_type, const enum can_interact_endianness byte_order, uint8_t* dest_array)
{
	return data_type == DATA_TYPE_FLOAT
		? _p_can_interact_serialise_float(host_number, len, byte_order, dest_array)
		: _p_can_interact_serialise_ints(host_number, len, byte_order, dest_array);
}

/**
 * @brief _p_can_interact_assemble_frame - INTERNAL METHOD. initialises can frame using provided data to fill out properties
 * @param const canid_t - desired can id for the provided data
 * @param const uint8_t - length of data
 * @param struct can_frame* - pointer to LINUX can frame to write values to
 */
static void _p_can_interact_assemble_frame(const canid_t desired_id, const uint8_t byte_len, struct can_frame* can_frame)
{
	can_frame->can_id = desired_id;
	can_frame->can_dlc = byte_len;
}

int can_interact_encode(const canid_t desired_id, const void* host_number, const uint8_t len, const enum can_interact_data_type data_type, const enum can_interact_endianness byte_order, struct can_frame* frame)
{
	const uint8_t res = _p_can_interact_serialise(host_number, len, data_type, byte_order, frame->data);
	if (res == 0) { /* i.e. not a single byte failed to encode */
		return 1;
	}
	_p_can_interact_assemble_frame(desired_id, res, frame);
	return 0;
}

int can_interact_send_frame(const struct can_frame* can_frame, const int* socket)
{
	int res = write(*socket, can_frame, sizeof(struct can_frame)) != sizeof(struct can_frame); /* 0 = success, 1 = no data reading from CAN */
	return res == 0 ? 0 : (int)errno;
}

int can_interact_fini(const int* socket)
{
	close(*socket);
	return (int)errno;
}
