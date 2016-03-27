/**
 * Connection handlers
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
#include <errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "sensorNetwork.h"
#include "base.h"
#include "node.h"

/*-----------------------------------------------------------------------------
 * File-scope constants & globals
 * --------------------------------------------------------------------------*/

#define PROXY_BACKLOG   11  /* How many pending connections the socket queue will hold */

/* Each node has to keep track of socket file descriptors for the other 10 nodes */
int sock_fd[NUM_TOTAL_NODES];  /* Indexed by node ID */

/*-----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

/**
 * Setup this process' TCP connections with the other ten nodes
 */
bool setup_peer_tcp_conns(uint8_t id)
{
    struct addrinfo hints;
    struct addrinfo *peer_info;
    struct addrinfo *p;
    int peer_sock = -1;
    int rv;
    int i;
    
    for (i = 0; i < NUM_TOTAL_NODES; i++)
    {
        /* Only establilsh connections with nodes that aren't yourself */
        if (i != id)
        {
            /* Clear out the hints struct for this new conn */
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            
            /* Use the hard-coded hostname and port numbers to get address info */
            if ((rv = getaddrinfo(HOSTNAME, NODE_PORT[i], &hints, &peer_info)) != 0)
            {
                fprintf(stderr, "Node %u getaddrinfo: %s\n", id, gai_strerror(rv));
                
                return false;
            }
            
            /* peer_info now points to a linked list of 1 or more struct addrinfos
               Loop through the list and bind to the first one we can */
            for(p = peer_info; p != NULL; p = p->ai_next)
            {
                if ((peer_sock = socket(p->ai_family, p->ai_socktype,
                                        p->ai_protocol)) == -1)
                {
                    fprintf(stderr, "Node %u Client: socket failed on node %u\n", id, i);
                    
                    continue;
                }

                if (connect(peer_sock, p->ai_addr, p->ai_addrlen) == -1)
                {
                    close(peer_sock);
                    fprintf(stderr, "Node %u Client: connect failed on node %u. Error: %s\n",
                            id, i, strerror(errno));
                    
                    continue;
                }

                break;
            }
            
            if (p == NULL)
            {
                fprintf(stderr, "Node %u Client: failed to connect to node %u\n", id, i);
                
                return false;
            }
            
            /* Store the socket file descriptor for this peer node */
            sock_fd[i] = peer_sock;
            
            freeaddrinfo(peer_info); /* No longer need this list */
        }
    }
    
    return true;
}


/**
 * Setup this process' TCP connection (get a socket file descriptor to listen on)
 */
bool setup_this_tcp_conn(uint8_t id)
{
    int status;
    int this_sock_fd;
    struct addrinfo hints;
    struct addrinfo *local_serv_info;
    struct addrinfo *cur_info;       /* Used to iterate through the server_info list */
    int reuse_addr = 1; /* True */
    
    /* Get the address info */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     /* Let getaddrinfo() chose an address for me */
    
    /* Get address info using the hard-coded port number allocated for this node ID */
    if ((status = getaddrinfo(NULL, NODE_PORT[id], &hints, &local_serv_info)) != 0)
    {
        fprintf(stderr, "Node %u getaddrinfo error: %s\n", id, gai_strerror(status));
        
        return false;
    }
    
    /* local_serv_info now points to a linked list of 1 or more struct addrinfos
       Loop through the list and bind to the first one we can */
    for(cur_info = local_serv_info; cur_info != NULL; cur_info = cur_info->ai_next)
    {
        if ((this_sock_fd = socket(cur_info->ai_family, cur_info->ai_socktype,
                              cur_info->ai_protocol)) == -1)
        {
            fprintf(stderr, "Node %u server: socket failure\n", id);
            
            continue;
        }

        if (setsockopt(this_sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) == -1)
        {
           fprintf(stderr, "Node %u setsockopt failure\n", id);
            
            return false;
        }

        if (bind(this_sock_fd, cur_info->ai_addr, cur_info->ai_addrlen) == -1)
        {
            close(this_sock_fd);
            fprintf(stderr, "Node %u server: bind failure", id);
            
            continue;
        }

        break;
    }
    
    freeaddrinfo(local_serv_info); /* Free the linked-list */
    
    /* Anounce that this socket will be used to listen for incoming connections */
    if (listen(this_sock_fd, PROXY_BACKLOG) == -1)
    {
        fprintf(stderr, "Node %u listen failure\n", id);
        
        return false;
    }
    
    sock_fd[id] = this_sock_fd;
    
    return true;
}


/**
 * Sends the node's state information to all other sensor nodes
 * 
 * *NOTE This does not include the base station since the only needs to receive
 *       data packets, not send them, therefore it doesnt need to know about other
 *       node's states
 */
bool send_node_state(uint8_t id, struct node_state state)
{
    int i;
    int num_bytes_written;
    struct msg_header out;
    uint32_t state_size;
    
    state_size = sizeof(state);
    
    out.type = STATE;
    out.msg_size = state_size;
    
    /* Allocate space in the header for the state message */
    out.msg = calloc(1, state_size);
    memcpy(out.msg, &state, state_size);
    
    /* Start at 1 since we don't worry about sending to the base */
    for (i = 1; i < NUM_TOTAL_NODES; i++)
    {
        if (i != id)
        {
            /* Then write it to the receiver's file descriptor */
            num_bytes_written = write(sock_fd[i], &out, sizeof(out));
            if (num_bytes_written == -1)
            {   
                fprintf(stderr, "Node %i ERROR writing to receiver %i socket\n", id, i);
                
                return false;
            }
            
            if (id == 1)
            {
                printf("Sending state to %i\n", i);
            }
        }
    }
    
    return true;
}
