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
#include "connection.h"

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
 * Helper functions
 * --------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

void run_base(uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n)
{   
    
    /* Save the global operation parameters */
    K = k;
    D = d;
    R = r;
    P = p;
    N = n;
    
    /* Setup this node's tcp connection */
    if (!setup_this_tcp_conn(BASE_ID))
    {
        fprintf(stderr, "Node %u failed to setup own TCP connection\n", BASE_ID);
        
        return;
    }
    
    /* Then get the TCP connection info for each of the other nodes */
    if (!setup_peer_tcp_conns(BASE_ID))
    {
        fprintf(stderr, "Node %u failed to setup TCP connections with other nodes\n", BASE_ID);
        
        return;
    }
    
    
    while(1);
    
}


