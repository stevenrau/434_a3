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
#include <time.h>

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

/* This node's state information */
struct node_state my_state;

/* This node's data packet */
struct data_packet my_data;


/*-----------------------------------------------------------------------------
 * Helper functions
 * --------------------------------------------------------------------------*/

void init_node_state()
{
    my_state.id = ID;
    
    /* Randomize the x and y coords */
    srand(ID);  /* Seed the generator with node ID to ensure unique values */
    my_state.x_pos = rand() % (REGION_X_MAX + 1);
    my_state.y_pos = rand() % (REGION_Y_MAX + 1);
    
    /* Zero out the has_data array and mark that we have data for this node */
    memset(my_state.has_data, 0, sizeof(bool) * NUM_TOTAL_NODES);
    my_state.has_data[ID] = true;
}

void init_node_data()
{
    strcpy(my_data.name, NODE_NAME[ID]);
    strcpy(my_data.text, "This is node data");
}

void move_node()
{
    int32_t new_x;
    int32_t new_y;
    
    uint8_t dir = rand() % 4;
    
    /* 0 is move right */
    if (dir == 0)
    {
        new_x = my_state.x_pos + D;
        
        /* If we went past the boudary, go back how ever far we went over */
        if (new_x > REGION_X_MAX)
        {
            new_x = REGION_X_MAX - (new_x - REGION_X_MAX);
        }
        
        my_state.x_pos = new_x;
    }
    /* 1 is move left */
    else if (dir == 1)
    {
        new_x = my_state.x_pos - D;
        
        /* If we went past the boudary, go back how ever far we went over */
        if (new_x < REGION_X_MIN)
        {
            new_x = REGION_X_MIN + (REGION_X_MIN - new_x);
        }
        
        my_state.x_pos = new_x;
    }
    /* 2 is move up */
    else if (dir == 2)
    {
        new_y = my_state.y_pos + D;
        
        /* If we went past the boudary, go back how ever far we went over */
        if (new_y > REGION_Y_MAX)
        {
            new_y = REGION_Y_MAX - (new_y - REGION_Y_MAX);
        }
        
        my_state.y_pos = new_y;
        
    }
    /* 3 is move down */
    else
    {
        new_y = my_state.y_pos - D;
        
        /* If we went past the boudary, go back how ever far we went over */
        if (new_y < REGION_Y_MIN)
        {
            new_y = REGION_Y_MIN + (REGION_Y_MIN - new_y);
        }
        
        my_state.y_pos = new_y;
    }
}

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

/**
 * Main sensor node starup function that gets called directly after the new sensor node
 * process is forked
 */
void run_sensor_node(uint8_t id,uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n)
{   
    int num_turns = 0;
    int peer_id;
    
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
    
    /* Initialize the state information and data packet*/
    init_node_state();
    init_node_data();
    
    printf("Node %i ready to go\n", id);
    
    for (num_turns = 0; num_turns < K; num_turns++)
    {
        if (id == 1)
        {
            printf("Turn %i\n", num_turns);
        }
        /* First, move in a random direction */
        move_node();
        
        /* Send your state to all other sensor nodes */
        if (!send_node_state(ID, my_state))
        {
            fprintf(stderr, "Node %u failed to send state info to other nodes\n", id);
        
            return;
        }
        
        /* Handle state messages from each of the other nodes */
        for (peer_id = 0; peer_id < NUM_TOTAL_NODES; peer_id++)
        {
            /* Only try to get messages from nodes other than yourself */
            if (peer_id != ID)
            {
                if (!handle_msg(id, peer_id, my_state, D))
                {
                    fprintf(stderr, "Node %u failed to handle message\n", id);
            
                    return;
                }
            }
        }
        
        /* Give the other processes time to catch up */
        sleep(1);
    }
}
