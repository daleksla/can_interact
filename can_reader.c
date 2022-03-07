#include <stdio.h> // io
#include <unistd.h> // syscalls
#include <stdlib.h> // std c lib
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>

/**
  * @brief Library header of can_reader system
  * File in nearly all cases designed and works as independant from any reading specifications etc..
  */

int can_socket_init(const char* net_device)
{
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW) ; // creates communication endpoint to a given data source. request for raw network protocol access
    if(s == -1)
    {
        fprintf(stderr, "Error opening initial socket endpoint\n") ;
        abort() ;
    }

    struct ifreq ifr ;
    strcpy(ifr.ifr_name, "vcan0") ;
    ioctl(s, SIOCGIFINDEX, &ifr) ;

    struct sockaddr_can addr ;
    memset(&addr, '\0', sizeof(addr)) ;
    addr.can_family = AF_CAN ;
    addr.can_ifindex = ifr.ifr_ifindex ;

    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) // bind the socket to the CAN Interface
    {
        fprintf(stderr, "Error binding socket to CAN\n") ;
        abort() ;
    }

    return s ;
}

void apply_can_fitler(const unsigned int* filter_ids, const size_t filter_id_len, const int socket)
{
    struct can_filter filter[filter_id_len] ;

    for(size_t i = 0 ; i < filter_id_len ; ++i)
    {
        filter[i].can_id = filter_ids[i] ;
        filter[i].can_mask = 0x1FFFFFFF ; // every bit must match filter (see https://www.cnblogs.com/shangdawei/p/4716860.html)
    }

    setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) ;
}

float hex_bytes_to_number(const uint8_t* payload, const size_t data_len, const bool byte_order, const bool signedness)
{
    union {
        uint8_t src[data_len] ;

        float dst ;
    } ulf ;

    for(size_t i = 0 ; i < data_len ; ++i)
    {
        if(byte_order) // little
        {
            ulf.src[(data_len-1)-i] = payload[i] ;
        }
        else { // big
            ulf.src[i] = payload[i] ;
        }
    }

    return ulf.dst ;
}
