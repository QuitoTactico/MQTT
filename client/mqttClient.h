#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BROKER_PORT "1883"
#define BROKER_IP "54.92.146.43"    // IP broker Esteban "54.92.146.43" | IP broker Miguel "54.196.182.249"

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

void createConnect(char *message);

//================================================================================================================

/*******************************************/
/*                                         */
/*                PUBLISH                  */
/*                                         */
/*******************************************/

void createPublish(char *message);

//================================================================================================================

/*******************************************/
/*                                         */
/*               SUBSCRIBE                 */
/*                                         */
/*******************************************/

void createSubscribe(char *message);

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