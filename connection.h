/**
 * Shared functions for handling TCP connections
 * 
 * CMPT 434 - A3
 * Steven Rau
 * scr108
 * 11115094
 */

#include <stdbool.h>

#include "sensorNetwork.h"
#include "base.h"
#include "node.h"

#ifndef __CONNECTION_H
#define __CONNECTION_H

/*-----------------------------------------------------------------------------
 * Constants and type defs
 * --------------------------------------------------------------------------*/

#define PROXY_BACKLOG   11  /* How many pending connections the socket queue will hold */

/* Array holding socket file descriptors for each of the nodes.
   Defined and set in connection.c */
extern int sock_fd[NUM_TOTAL_NODES];

/*-----------------------------------------------------------------------------
 * Function headers
 * --------------------------------------------------------------------------*/

bool setup_peer_tcp_conns(uint8_t id);

bool setup_this_tcp_conn(uint8_t id);

bool send_node_state(uint8_t id, struct node_state state);


#endif  /* __CONNECTION_H */