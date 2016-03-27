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

/*-----------------------------------------------------------------------------
 * Constants and type defs
 * --------------------------------------------------------------------------*/

/* The base station always has the node ID 0 */
#define BASE_ID  0

/* The base station remains in the same place */
#define BASE_POS_X  500
#define BASE_POS_Y  500


/*-----------------------------------------------------------------------------
 * Function headers
 * --------------------------------------------------------------------------*/

void run_base(uint16_t k, uint16_t d, uint32_t r, uint16_t p, int8_t n);

#endif  /* __BASE_H */