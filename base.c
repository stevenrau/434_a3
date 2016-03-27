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

/* The base node's state information */
struct node_state my_state;


/*-----------------------------------------------------------------------------
 * Helper functions
 * --------------------------------------------------------------------------*/

void init_base_state()
{
    my_state.id = BASE_ID;
    
    /* Set the x and y coords */
    my_state.x_pos = BASE_POS_X;
    my_state.y_pos = BASE_POS_Y;
    
    /* Zero out the has_data array and mark that we have data for this node */
    memset(my_state.has_data, 0, sizeof(bool) * NUM_TOTAL_NODES);
    my_state.has_data[BASE_ID] = true;
}

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

void run_base(uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n)
{   
    int num_turns = 0;
    
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
    
    /* Initialize the state information */
    init_base_state();
    
    printf("Node %i ready to go\n", BASE_ID);
    
    
    for (num_turns = 0; num_turns < K; num_turns++)
    {
        /* Send your state to all other sensor nodes */
        if (!send_node_state(BASE_ID, my_state))
        {
            fprintf(stderr, "Node %u failed to send state info to other nodes\n", BASE_ID);
        
            return;
        }
        
        /* Give the other processes time to catch up */
        sleep(1);
    }
    
    while(1);
}


