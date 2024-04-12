#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define MY_PORT "1883"
#define MY_IP ""
#define QUEUESIZE 10



//================================================================================================================

/*******************************************/
/*                                         */
/*            HANDLING MESSAGES            */
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



//================================================================================================================

/*******************************************/
/*                                         */
/*                DATABASE                 */
/*                                         */
/*******************************************/

// General

int DBsaveStringsToFile(const char* filename, const char* string1, const char* string2);

int DBverifySession(const char* username, const char* password);

int DBcheckExistence(const char* filename, const char* string1);

int DBupdateOrCreate(const char* filename, const char* string1, const char* string2);

// Subscriptions

int DBgetSubscribes(char* username, char*** topics);

int DBisUserInList(char* username, char** users, int usersCount);

int DBgetSubscriptors(char* topic, char*** users);

// Sockets

int DBgetSocketByUsername(const char* username);

// Log

void DBsaveLog(char* dir, char* ip, char* request, char* args);
