/**
 * Public base station data
 * 
 * CMPT 434 - A3
 * Steven Rau
 * scr108
 * 11115094
 */

#ifndef __BASE_H
#define __BASE_H

#include <stdint.h>

#include "sensorNetwork.h"

/*-----------------------------------------------------------------------------
 * Constants and type defs
 * --------------------------------------------------------------------------*/

/* The base station always has the node ID 0 */
#define BASE_ID  0

/* The base station remains in the same place */
#define BASE_POS_X  500
#define BASE_POS_Y  500

/* Define an array to hold recieved data packets large enough to potentially hold all node data */
extern struct data_packet data_recvd[NUM_TOTAL_NODES];

/* Number of data packets we currently have received */
extern uint8_t num_recvd;


/*-----------------------------------------------------------------------------
 * Function headers
 * --------------------------------------------------------------------------*/

void run_base(void);

#endif  /* __BASE_H */