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

/* The base node's state information */
struct node_state my_state;

/* Extern data packet array to hold any data pakets we receive from nodes */
struct data_packet data_recvd[NUM_TOTAL_NODES];

/* Extern count of the number of data packets we currently have received */
uint8_t num_recvd = 0;


/*-----------------------------------------------------------------------------
 * Helper functions
 * --------------------------------------------------------------------------*/

void init_base_state()
{
    my_state.id = BASE_ID;
    
    /* Set the x and y coords */
    my_state.x_pos = BASE_POS_X;
    my_state.y_pos = BASE_POS_Y;
}

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

void run_base()
{   
    int peer_id;
    int i;
    int num_turns = 0;
    
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
        /* Print debug info */
        if (BASE_ID == N)
        {
            printf("DEBUG: Turn %i\n", num_turns);
            printf("\tX coord: %u  Y coord: %u\n", my_state.x_pos, my_state.y_pos);
        }
        
        /* Send your state to all other sensor nodes */
        if (!send_node_state(BASE_ID, my_state))
        {
            fprintf(stderr, "Node %u failed to send state info to other nodes\n", BASE_ID);
        
            return;
        }
        
        /* Handle state messages from each of the other nodes */
        for (peer_id = 0; peer_id < NUM_TOTAL_NODES; peer_id++)
        {
            /* Only try to get messages from nodes other than yourself */
            if (peer_id != BASE_ID)
            {
                if (!base_handle_msg(peer_id, my_state))
                {
                    fprintf(stderr, "Node %u failed to handle message\n", BASE_ID);
            
                    return;
                }
            }
        }
        
        /* Give the other processes time to catch up */
        sleep(2);
    }
    
    close_connections();
    
    printf("\nComplete!\n"
           "The base station received %u data packets\n", num_recvd);
    
    for (i = 0; i < num_recvd; i++)
    {
        printf("\t%s: %s\n", data_recvd[i].name, data_recvd[i].text);
    }
    
    printf("\nPress ctrl + c to exit the program (otherwise children will loop forever)\n");

}


