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

/* Each node has to keep track of socket file descriptors for sending to the other 10 nodes */
int out_sock_fd[NUM_TOTAL_NODES];  /* Indexed by node ID */

/* Each node also has an incoming file descriptor given by accept() */
int in_sock_fd[NUM_TOTAL_NODES];

/* Keep track of the number of conenctions we've accpeted so we know whe to stop trying to accept */
int num_accepted = 0;

/*-----------------------------------------------------------------------------
 * Internal helper functions
 * --------------------------------------------------------------------------*/

/**
 * Checks to see if this node can afford to send a data packet
 */
bool can_afford_send()
{
    /* No if we've already sent our limit */
    if (packs_sent >= P)
    {
        return false;
    }
    /* No if we might not be able to send all buffered packets after this round 
       (+2 since in worst case, we send one packet and receive one that we buffer to send later) */
    else if ((packs_sent + num_buffered + 2) >= P)
    {
        return false;
    }
    
    return true;
}

/**
 * Adds a data packet to the base station's received data array, or ignores it if already
 * received
 */
void add_data_to_base(struct data_packet in_data)
{
    int i;
    
    /* Check and see if we already have this data packet */
    for (i = 0; i < num_recvd; i++)
    {
        if (0 == strcmp(data_recvd[i].name, in_data.name))
        {
            return;
        }
    }
    
    /* If not, add it */
    memcpy(&data_recvd[i], &in_data, sizeof(struct data_packet));
    num_recvd++;
}

/**
 * Handles data packets incoming to the base node
 */
bool base_handle_incoming_data(int peer_fd)
{
    int i;
    int num_bytes_read;
    num_incoming_data num_expected;
    struct data_packet in_data;
    
    /* Read in the incoming number of data packets to expect */
    num_bytes_read = read(peer_fd, &num_expected, sizeof(num_expected));
    
    if (num_bytes_read < 1) 
    {
        fprintf(stderr, "Node %i ERROR reading from sender socket\n", BASE_ID);
        
        return false;
    }
    
    for (i = 0; i < num_expected; i++)
    {
        /* Read in the the expected data packet */
        num_bytes_read = read(peer_fd, &in_data, sizeof(struct data_packet));
        
        if (num_bytes_read < 1) 
        {
            fprintf(stderr, "Node %i ERROR reading from sender socket\n", BASE_ID);
            
            return false;
        }
        
        add_data_to_base(in_data);
    }
    
    return true;
}

/*-----------------------------------------------------------------------------
 * Externally vidible functions
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
            out_sock_fd[i] = peer_sock;
            
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
    
    out_sock_fd[id] = this_sock_fd;
    in_sock_fd[id] = this_sock_fd;
    
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
    out.id = id;
    out.msg_size = state_size;
    
    /* Allocate space in the header for the state message */
    out.msg = calloc(1, state_size);
    memcpy(out.msg, &state, state_size);
    
    /* Start at 1 since we don't worry about sending to the base */
    for (i = 0; i < NUM_TOTAL_NODES; i++)
    {
        if (i != id)
        {
            /* Then write it to the receiver's file descriptor */
            num_bytes_written = write(out_sock_fd[i], &state, sizeof(state));
            if (num_bytes_written == -1)
            {   
                fprintf(stderr, "Node %i ERROR writing to receiver %i socket\n", id, i);
                
                return false;
            }
        }
    }
    
    return true;
}

/**
 * Handles a state message received by the base from a peer node and acts accordingly
 */
bool base_handle_msg(uint8_t peer_id, struct node_state state)
{
    struct sockaddr_in6 client_addr; /* Client's address information */
    socklen_t sin_size = sizeof(struct sockaddr_in6);
    int peer_fd;
    int num_bytes_read;
    struct node_state in_state;
    
    /* Accept new connections as long as we haven't accepted from all peer nodes */
    if (num_accepted < NUM_SENSOR_NODES)
    {
        peer_fd = accept(out_sock_fd[BASE_ID], (struct sockaddr *)&client_addr, &sin_size);
        if (peer_fd == -1)
        {
            fprintf(stderr, "Node %i accept error", BASE_ID);
                
            return false;
        }
        
        num_accepted++;
    }
    /* Otherwise grab the FD we already know for the current node */
    else
    {
        peer_fd = in_sock_fd[peer_id];
    }
    
    /* Read in the peer node's state info */
    num_bytes_read = read(peer_fd, &in_state, sizeof(struct node_state));
    
    if (num_bytes_read < 1) 
    {
        fprintf(stderr, "Node %i ERROR reading from sender socket\n", BASE_ID);
        
        return false;
    }
    
    /* Set the FD for this peer node so that we know for future use */
    in_sock_fd[in_state.id] = peer_fd;
    
    
    /* Check if the nodes are within the range R */
    if (test_distance(state.x_pos, state.y_pos, in_state.x_pos, in_state.y_pos))
    {
        if (BASE_ID == N)
        {
            printf("\tNode %i Within transmission distance of node %i\n", BASE_ID, in_state.id);
        }
        
        /* Handle any potential incoming data packets */
        return base_handle_incoming_data(peer_fd);
    }
    
    return true;
}

/**
 * Handles a state message received by a node from a peer node and acts accordingly
 */
bool node_handle_msg(uint8_t my_id, uint8_t peer_id, struct node_state state)
{
    struct sockaddr_in6 client_addr; /* Client's address information */
    socklen_t sin_size = sizeof(struct sockaddr_in6);
    int peer_fd;
    int num_bytes_read;
    struct node_state in_state;
    
    /* Accept new connections as long as we haven't accepted from all peer nodes */
    if (num_accepted < NUM_SENSOR_NODES)
    {
        peer_fd = accept(out_sock_fd[my_id], (struct sockaddr *)&client_addr, &sin_size);
        if (peer_fd == -1)
        {
            fprintf(stderr, "Node %i accept error", my_id);
                
            return false;
        }
        
        num_accepted++;
    }
    /* Otherwise grab the FD we already know for the current node */
    else
    {
        peer_fd = in_sock_fd[peer_id];
    }
    
    /* Read in the peer node's state info */
    num_bytes_read = read(peer_fd, &in_state, sizeof(struct node_state));
    
    if (num_bytes_read < 1) 
    {
        fprintf(stderr, "Node %i ERROR reading from sender socket\n", my_id);
        
        return false;
    }
    
    /* Set the FD for this peer node so that we know for future use */
    in_sock_fd[in_state.id] = peer_fd;
    
    
    /* Check if the nodes are within the range R */
    if (test_distance(state.x_pos, state.y_pos, in_state.x_pos, in_state.y_pos))
    {
        if (my_id == N)
        {
            printf("\tNode %i Within transmission distance of node %i\n", my_id, in_state.id);
        }
        
        /* If we are within range of the abse station, send all buffered data and continue.
           No need to wait since the base station will not be sending us anything */
        if (in_state.id == BASE_ID)
        {
            
        }
        /* Else decide if we will send packet or not and recieve the other node's potential data */
        else
        {
            /* If we haven't already sent our data to this node and we can afford a data transmission,
            send your data */
            if (!data_sent[in_state.id] && can_afford_send())
            {
                
            }
            /* Else notify that we won't be sending any data */
            else
            {
                
            }
            
            /* Handle the peer node'es data (or lack of) */
        }
    }
    
    return true;
}

/**
 * Closes connections with all open file descriptors
 */
void close_connections()
{
    
}
