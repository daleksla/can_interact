#define _DEFAULT_SOURCE

#include <unistd.h> /* syscalls */
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <linux/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>
#include <endian.h>

#include "can_interact.h"

/**
  * @brief Functionality definitions of library code to be used to usefully read and write to CAN bus
  * File in nearly all cases designed and works as independant from any reading specifications etc..
  */

int can_socket_init(const char* net_device)
{
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW) ; /* creates communication endpoint to a given data source. request for raw network protocol access */
    if(s == -1)
    {
        return -1 ;
    }

    struct ifreq ifr ;
    strcpy(ifr.ifr_name, net_device) ;
    ioctl(s, SIOCGIFINDEX, &ifr) ;

    struct sockaddr_can addr ;
    memset(&addr, '\0', sizeof(addr)) ;
    addr.can_family = AF_CAN ;
    addr.can_ifindex = ifr.ifr_ifindex ;

    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) /* bind the socket to the CAN Interface */
    {
        return -1 ;
    }

    return s ;
}

void apply_can_fitler(const unsigned int* filter_ids, const size_t filter_id_len, const int socket)
{
    struct can_filter filter[filter_id_len] ;

    size_t i ;
    for(i = 0 ; i < filter_id_len ; ++i)
    {
        filter[i].can_id = filter_ids[i] ;
        filter[i].can_mask = 0x1FFFFFFF ; /* every bit must match filter (see https://www.cnblogs.com/shangdawei/p/4716860.html) */
    }

    setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) ;
}

int get_can_frame(struct can_frame* can_frame, const int socket)
{
    const int nbytes = read(socket, can_frame, sizeof(struct can_frame)) ;
    return nbytes <= 0 ? 1 : 0 ; /* 0 = success, 1 = no data reading from CAN */
}

uint64_t hex_bytes_to_number(const uint8_t* payload, const size_t data_len, const enum SignedType is_signed, const enum EndianType byte_order)
{
    uint64_t result = 0 ; /* [0,0,0,0,0,0,0,0] */
    uint8_t* blocks = (uint8_t*)(&result) ; /* array of 8 */

    size_t i ;
    if(byte_order == LITTLE_ENDIAN_VAL)
    {
        for(i = 0 ; i < (is_signed ? data_len - 1 : data_len) ; ++i)
        {
            blocks[i] = payload[i] ;
        }
        /* if data_len is two and unsigned, then [i,i+1,0,0,0,0,0,0]
         * if data_len is two and signed, then [i,0,0,0,0,0,0,i+1] */
        blocks[7] = is_signed ? payload[data_len - 1] : blocks[7] ; /* if data is signed, use last byte which will be MSB in little endian
                                                               assign that to end of MSB (assuming little endian again) of 64 byte num */
        result = le64toh(result) ; /* little endian->host byte order */
    }
    else if(byte_order == BIG_ENDIAN_VAL)
    {
        for(i = (is_signed ? 1 : 0) ; i < data_len ; ++i)
        {
            blocks[8 - data_len + i] = payload[i] ;
        }
        /* if data_len is two, then [0,0,i,i+1] */
        blocks[0] = is_signed ? payload[0] : blocks[0] ; /* if data is signed, use first byte which will be MSB in big endian
                                                    assign that to start of MSB (assuming big endian again) of 64 byte num */
        result = be64toh(result) ; /* big endian->host byte order */
    }

    return result ;
}

size_t number_to_hex_bytes(const uint64_t host_number, uint8_t* dest_array, const enum SignedType is_signed, const enum EndianType byte_order)
{
    const size_t TOTAL_LENGTH = 8 ;
    memset(dest_array, 0, TOTAL_LENGTH) ; /* initialise array elements with value 0 */

    /* determine endianess of number provided by system : 0 for big endian, 1 for little endian */
    volatile uint16_t value = 0x4567 ;

    int earliest_consis_zero = -1 ;
    size_t i ;

    if(byte_order == LITTLE_ENDIAN_VAL)
    {
        const uint64_t le_val = htole64(host_number) ; /* convert number first from host order->little endian */
        uint8_t* blocks = (uint8_t*)(&le_val) ; /* array of 8 */

        for(i = 0 ; i < (is_signed ? TOTAL_LENGTH - 1 : TOTAL_LENGTH) ; ++i)
        {
            dest_array[i] = blocks[i] ;

            if(dest_array[i] == 0) /* if the current byte is 0 */
            {
                if(earliest_consis_zero == -1)
                {
                    earliest_consis_zero = i ;
                }
            }
            else {
                earliest_consis_zero = -1 ;
            }
        }

        if(is_signed)
        {
             dest_array[earliest_consis_zero != -1 ? earliest_consis_zero++ : TOTAL_LENGTH - 1] = \
                blocks[TOTAL_LENGTH - 1] ;
        }
    }
    else if(byte_order == BIG_ENDIAN_VAL)
    {
        const uint64_t be_val = htobe64(host_number) ; /* convert number first from host order->big endian */
        uint8_t* blocks = (uint8_t*)(&be_val) ; /* array of 8 */

        if(is_signed)
        {
             dest_array[0] = blocks[0] ;
        }

        for(i = (is_signed ? 1 : 0) ; i < TOTAL_LENGTH ; ++i)
        {
            dest_array[i] = blocks[i] ;

            if(dest_array[i] == 0) /* if the current byte is 0 */
            {
                if(earliest_consis_zero == -1)
                {
                    earliest_consis_zero = i ;
                }
            }
            else {
                earliest_consis_zero = -1 ;
            }
        }
    }

    return earliest_consis_zero == -1 ? TOTAL_LENGTH : earliest_consis_zero ;
}

void create_can_frame(const canid_t desired_id, const uint8_t* bytes, const size_t byte_len, struct can_frame* can_frame)
{
    can_frame->can_id = desired_id ;
    can_frame->can_dlc = byte_len ;
    memcpy(can_frame->data, bytes, byte_len) ;
}

int send_can_frame(const struct can_frame* can_frame, const int socket)
{
    return (write(socket, can_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) ; /* 0 = success, 1 = no data reading from CAN */
}
