#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BROKER_PORT "1883"

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

int createConnect(char *message);
int handleConnack(char *connack);

//================================================================================================================

/*******************************************/
/*                                         */
/*                PUBLISH                  */
/*                                         */
/*******************************************/

void createPublish(char *message);
int handlePuback(char *connack);

//================================================================================================================

/*******************************************/
/*                                         */
/*               SUBSCRIBE                 */
/*                                         */
/*******************************************/

void createSubscribe(char *message);
int handleSuback(char *suback);

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

// create the socket file descriptor with the ip, port and queue size
int connectSocket(char* port, char* ip);

// prints the info of the server socket and the client socket
int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage* their_addr, socklen_t addr_size);

int handleRequest(void *args);

int handleRecv(void * sockfd);