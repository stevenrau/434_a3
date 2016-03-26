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

/*-----------------------------------------------------------------------------
 * Constants and type defs
 * --------------------------------------------------------------------------*/

/* Number of nodes that get forked */
#define NUM_SENSOR_NODES  10

/* Total number of nodes (base station + 10 sensor nodes) */
#define NUM_TOTAL_NODES   (NUM_SENSOR_NODES + 1)

/* Since we have a constant number of nodes and base stations, hard code their names,
   indexed by their IDs (0 for base station, 1-10 for sensor nodes) */
extern const char* NODE_NAME[];

extern char HOSTNAME[256];
                           
                           
#endif  /* __SENSOR_NETWORK_H */