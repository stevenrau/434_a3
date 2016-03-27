/**
 * Program-wide shared data
 * 
 * CMPT 434 - A3
 * Steven Rau
 * scr108
 * 11115094
 */

#ifndef __SENSOR_NETWORK_H
#define __SENSOR_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*-----------------------------------------------------------------------------
 * Constants
 * --------------------------------------------------------------------------*/

/* Number of nodes that get forked */
#define NUM_SENSOR_NODES  10

/* Total number of nodes (base station + 10 sensor nodes) */
#define NUM_TOTAL_NODES   (NUM_SENSOR_NODES + 1)

/* Define the region boundaries */
#define REGION_X_MAX  1000
#define REGION_X_MIN  0
#define REGION_Y_MAX  1000
#define REGION_Y_MIN  0

/* Since we have a constant number of nodes and base stations, hard code their names,
   indexed by their IDs (0 for base station, 1-10 for sensor nodes) */
extern const char* NODE_NAME[];

/* Hard-code a port number for each node and make it externally visible */
extern const char* NODE_PORT[];

extern char HOSTNAME[256];

/*-----------------------------------------------------------------------------
 * Data types
 * --------------------------------------------------------------------------*/

/* There are two types of messages, node state, and node data */
typedef enum {STATE, DATA} msg_type;

struct msg_header
{
    msg_type type;
    uint8_t id;
    uint32_t msg_size;
    void *msg; /* Holds either a data_packet or node_state message */
};

/* The data packet sent by the nodes. Contains the node name and a short text string */
struct data_packet
{
    char name[4];
    char text[64];
};

/* Each node first passes it's current state to all other nodes */
struct node_state
{
    uint8_t id;
    uint16_t x_pos;
    uint16_t y_pos;
    bool has_data[NUM_TOTAL_NODES];  /* True if this node has the data for node indexed by ID */
};

/*-----------------------------------------------------------------------------
 * Function headers
 * --------------------------------------------------------------------------*/

bool test_distance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t D);
                           
                           
#endif  /* __SENSOR_NETWORK_H */