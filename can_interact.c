#define _DEFAULT_SOURCE

#include <unistd.h> /* syscalls */
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <endian.h>

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
 * @brief Functionality definitions of library code to be used to usefully read and write to CAN bus
 * File in nearly all cases designed and works as independant from any reading specifications etc..
 */

int can_interact_init(int *s, const char *net_device)
{
	struct ifreq ifr; /* used to configure net device */
	struct sockaddr_can addr; /* assigns address connections */

	*s = socket(PF_CAN, SOCK_RAW, CAN_RAW); /* creates communication endpoint to a given data source. request for raw network protocol access */
	if(*s == -1) {
		return 1;
	}
	strcpy(ifr.ifr_name, net_device); /* copy name */
	ioctl(*s, SIOCGIFINDEX, &ifr); /*  manipulates the underlying device parameters of special files */
	memset(&addr, '\0', sizeof(addr)); /* clear existing struct */
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if(bind(*s, (struct sockaddr*)&addr, sizeof(addr)) == -1) { /* bind the socket to the CAN Interface */
		return 2;
	}

	return 0;
}

int can_interact_filter(const unsigned int *filter_ids, const size_t filter_id_len, const int *socket)
{
	struct can_filter *filters = alloca(sizeof(struct can_filter) * filter_id_len);
	size_t i;

	for(i = 0; i < filter_id_len; ++i) {
		filters[i].can_id = filter_ids[i];
		filters[i].can_mask = 0x1FFFFFFF; /* every bit must match filter (see https://www.cnblogs.com/shangdawei/p/4716860.html) */
	}
	return setsockopt(*socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filters, sizeof(filters));
}

int can_interact_get_frame(struct can_frame* can_frame, const int *socket)
{
	const ssize_t nbytes = read(*socket, can_frame, sizeof(struct can_frame));
	return nbytes <= 0 ? 1 : 0; /* 0 = success, 1 = no data reading from CAN */
}

uint64_t can_interact_decode(const uint8_t *payload, const uint8_t data_len, const enum can_interact_data_type is_signed, const enum can_interact_endianness byte_order)
{
	uint64_t result; /* [0,0,0,0,0,0,0,0] */
	uint8_t* blocks; /* array of 8 */
	result = 0;
	blocks = (uint8_t*)(&result);

	if (byte_order == ENDIAN_LITTLE) {
		memcpy(blocks, payload, data_len); /* copy data to start of number */
		if (is_signed && ((const int8_t*)payload)[data_len - 1] < 0) {
			memset(blocks + data_len, 0xFF, BYTE_MAX_LENGTH - data_len);
		}
		result = le64toh(result);
	} else if (byte_order == ENDIAN_BIG) {
		memcpy(blocks + (BYTE_MAX_LENGTH - data_len), payload, data_len); /* copy number from end - data_len */
		if (is_signed && ((const int8_t*)payload)[0] < 0) {
			memset(blocks, 0xFF, BYTE_MAX_LENGTH - data_len);
		}
		result = be64toh(result);
	} else { /* float */
		/* process float */
	}

	return result;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
uint8_t can_interact_encode(const uint64_t host_number, uint8_t* dest_array, const enum can_interact_data_type is_signed, const enum can_interact_endianness byte_order)
{
	uint64_t host_cpy ;
	
	if (byte_order == ENDIAN_LITTLE) {
		host_cpy = htole64(host_number); 
	} else if(byte_order == ENDIAN_BIG) {
		host_cpy = htobe64(host_number); 
	} else { /* float */
		/* process float */
	}

	memcpy(dest_array, (uint8_t*)&host_cpy, BYTE_MAX_LENGTH);

	return BYTE_MAX_LENGTH; /* for now, always just fill buffer */
}
#pragma GCC diagnostic pop

int can_interact_make_frame(const canid_t desired_id, const uint8_t* bytes, const uint8_t byte_len, struct can_frame* can_frame)
{
	if (byte_len > 8) {
		return 1;
	}

	can_frame->can_id = desired_id;
	can_frame->can_dlc = byte_len;
	memcpy(can_frame->data, bytes, byte_len);

	return 0 ;
}

int can_interact_send_frame(const struct can_frame* can_frame, const int* socket)
{
	return (write(*socket, can_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)); /* 0 = success, 1 = no data reading from CAN */
}
