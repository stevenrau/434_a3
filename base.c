/**
 * Base station process
 * 
 * CMPT 434 - A3
 * Steven Rau
 * scr108
 * 11115094
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "sensorNetwork.h"
#include "base.h"
#include "node.h"

/*-----------------------------------------------------------------------------
 * File-scope constants & globals
 * --------------------------------------------------------------------------*/

/* The base node needs to keep track of the command line params that were sent in */
uint16_t K;
uint16_t D;
uint32_t R;
uint16_t P;
int8_t N;

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

void run_base(uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n)
{
    printf("%s\n", NODE_NAME[BASE_ID]);
    printf("Hostname: %s\n", HOSTNAME);
    
    /* Save the global operation parameters */
    K = k;
    D = d;
    R = r;
    P = p;
    N = n;
}


