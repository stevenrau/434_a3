/**
 * Public Node data
 * 
 * CMPT 434 - A3
 * Steven Rau
 * scr108
 * 11115094
 */

#ifndef __NODE_H
#define __NODE_H

#include <stdint.h>
#include <stdbool.h>

#include "sensorNetwork.h"

/*-----------------------------------------------------------------------------
 * Constants and type defs
 * --------------------------------------------------------------------------*/

/* This node's data packet */
extern struct data_packet my_data;

/* Keeps track of which nodes you have sent your data packet to. Only need to send to each once, 
 * since they will either stil have your data buffered or have already transmitted to the base */
extern bool data_sent[NUM_TOTAL_NODES];

/* Keeps track of the number of data packets this node has sent. Cannot exceed P */
extern uint16_t packs_sent;

/* Define a data packet buffer large enough to potentially hold all node data */
extern struct data_packet data_buffer[NUM_TOTAL_NODES];

/* Number of data packets we currently have buffered */
extern uint8_t num_buffered;

/*-----------------------------------------------------------------------------
 * Function headers
 * --------------------------------------------------------------------------*/

void run_sensor_node(uint8_t id);


#endif  /* __NODE_H */