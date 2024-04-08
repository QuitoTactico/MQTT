#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MY_PORT "1883"
#define MY_IP ""
#define QUEUESIZE 1

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

// create the socket file descriptor with the ip, port and queue size
int createSocket(char* port, char* ip, int queue);

// prints the info of the server socket and the client socket
int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage* their_addr, socklen_t addr_size);

int handleRecv(void *args);

// handles the input to close the server
int handleServer(void *args);

// returns 0 if the server should close
int close_server();