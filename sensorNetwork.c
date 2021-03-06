/**
 * Main sensor network program that initiates the 10 nodes and single base station
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

#include "sensorNetwork.h"
#include "base.h"
#include "node.h"

/*-----------------------------------------------------------------------------
 * File-scope constants & globals
 * --------------------------------------------------------------------------*/

/* Define the constant node names that are externally visible to all nodes
 * (extern def in sensorNetwork.h) */
const char* NODE_NAME[] = { "N00",
                            "N01",
                            "N02",
                            "N03",
                            "N04",
                            "N05",
                            "N06",
                            "N07",
                            "N08",
                            "N09",
                            "N10" };

/* Also define the port numbers for each of the nodes to use (arbitrarily chosen to be
 * between 30000-40000, also has extern def in sensorNetwork.h) */
const char *NODE_PORT[] = { "35000",
                            "35001",
                            "35002",
                            "35003",
                            "35004",
                            "35005",
                            "35006",
                            "35007",
                            "35008",
                            "35009",
                            "35010" };
                           
/* Define the extern variable that holds the local hostname for all processes to use 
 * (extern def in sensorNetwork.h) */                           
char HOSTNAME[256];

/* extern variable holding command line params sent in. They get set here. */
uint16_t K;
uint16_t D;
uint32_t R;
uint16_t P;
int8_t N;

/*-----------------------------------------------------------------------------
 * Helper Functions
 * --------------------------------------------------------------------------*/

/**
 * Tests to see if to nodes (represented by x1, y1 and x2, y2) are withing distance
 * R of each other.
 * 
 * Use Euclidean distance.
 * 
 * Returns true if they are within R distance, false otherwise.
 */
bool test_distance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    float x_diff;
    float y_diff;
    float dist;
    
    x_diff = (float)x2 - (float)x1;
    y_diff = (float)y2 - (float)y1;
    
    dist = sqrt(pow(x_diff, 2) + pow(y_diff, 2));
    
    if (dist > (float)R)
    {
        return false;
    }
    
    return true;
}


/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/

/**
 * The main setup function for part A sensor network of assignment 3.
 * 10 node processes are forked from this process which execute the node code (node.c/h), 
 * while the original parent process continues with the base station code (base.c/h)
 * 
 * Expects values for K, D, R, and P as command line arguments: ./base K D R P N
 * 
 * @param[in] K  number of simulated time steps
 * @param[in] D  distance moved during each time step
 * @param[in] R  transmission range
 * @param[in] P  max number of data packet transmissions
 * @param[in] N  If between 0-10, specifies which base (0) or node (1-10) should print
 *               debug info
 */
int main(int argc, char **argv)
{
    int i = 0;
    int pid = 1;
    
    /* Get the server host and port from the command line */
    if (argc < 6)
    {
        fprintf(stderr, "Usage: %s <K> <D> <R> <P> <N>\n"
                        "\tK - number of simulated time steps\n"
                        "\tD - distance moved during each time step\n"
                        "\tR - transmission range\n"
                        "\tP - max number of data packet transmissions\n"
                        "\tN - 0-10 chooses a node to print debug info, -1 to ignore\n", 
                        argv[0]);
        
        exit(1);
    }
    
    /* Do parameter checks */
    if (atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0 || atoi(argv[4]) < 0)
    {
        
        exit(1);
    }
    else if (atoi(argv[5]) < -1 || atoi(argv[5]) > 10)
    {
        fprintf(stderr, "Usage: N must be between 0-10 or -1 for no debug printing\n");
        
        exit(1);
    }
    
    /* Grab the parameters now that we know they are acceptable */
    K = atoi(argv[1]);
    D = atoi(argv[2]);
    R = atoi(argv[3]);
    P = atoi(argv[4]);
    N = atoi(argv[5]);
    
    /* Get the hostname since all processes communicate on this local host (stored in extern) */
    gethostname(HOSTNAME, 256);
    
    printf("Sensor network started\n");
    
    /* Child processes will immediately exit the loop while the parent forks all 10 sensor
     * node children. i will be the sensor node's ID when it exits the loop */
    while (i < NUM_SENSOR_NODES && pid > 0)
    {
        i++;
        
        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Failed to fork sensor node\n");
        
            exit(1);
        }
    }
    
    /* If we are the parent (have pid of child process), run the base station code */
    if (pid != 0)
    {
        run_base();
    }
    /* Otherwise we are a child process (pid equals 0), so run the sensor node code */
    else
    {
        run_sensor_node(i);
    }
    
    return 0;
}
