/**
 * Node process
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
#include<time.h>

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

/* Each node needs to keep track of the command line params that were sent in */
uint16_t K;
uint16_t D;
uint32_t R;
uint16_t P;
int8_t N;

/* Each node also has a unique id */
uint8_t ID;

/* The position of the node in the region */
uint16_t x_pos;
uint16_t y_pos;


/*-----------------------------------------------------------------------------
 * Helper functions
 * --------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

/**
 * Main sensor node starup function that gets called directly after the new sensor node
 * process is forked
 */
void run_sensor_node(uint8_t id,uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n)
{   
    /* Save the global operation parameters */
    ID = id;
    K = k;
    D = d;
    R = r;
    P = p;
    N = n;
    
    /* Setup this node's tcp connection */
    if (!setup_this_tcp_conn(id))
    {
        fprintf(stderr, "Node %u failed to setup own TCP connection\n", id);
        
        return;
    }
    
    /* Wait 1 second before trying to connect to the other nodes. Make sure everyone has had
       enough time to establish their own connection */
    sleep(1);
    
    /* Then get the TCP connection info for each of the other nodes */
    if (!setup_peer_tcp_conns(id))
    {
        fprintf(stderr, "Node %u failed to setup TCP connections with other nodes\n", id);
        
        return;
    }
    
    printf("Node %i ready to go\n", id);
    
    while(1);
}
