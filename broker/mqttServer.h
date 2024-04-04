#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

#define MY_PORT "1883"
#define MY_IP "1270.1.0."
#define QUEUESIZE 1

//================================================================================================================

/*
    ====FIXE HEADER====

    it is always in there

    ====VARIABLE HEADER====

    PUBLISH
    PUBACK
    PUBREC
    PUBREL
    PUBCOMP
    SUBSCRIBE
    SUBACK
    UNSUBSCRIBE
    UNSUBACK

    ====PAYLOAD====

    CONNECT     required
    PUBLISH     optional
    SUBSCRIBE   required
    SUBACK      required
    UNSUBSCRIBE required
    UNSUBACK    required
*/

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

typedef struct {
    uint8_t messageType;
    uint16_t remainingLenght;
} fixedHeader;

/*******************************************/
/*                                         */
/*              CONTROL PACKET             */
/*                                         */
/*******************************************/

#define FIXED   0b11110000

#define CONNECT   0b00010000   // 1 || CONNECT     || CLIENT TO SERVER //
#define CONNACK   0b00100000   // 2 || CONNECT ACK || SERVER TO CLIENT
#define PUBLISH   0b00110000   // 3 || PUBLISH MESSAGE || BOTH WAYS
#define PUBACK    0b01000000   // 4 || PUBLISH ACK     || BOTH WAYS

// WILL NOT IMPLETEMT FOR NOW
#define PUBREC    0b01010000   // 5 || PUBLISH RECIEVE  || BOTH WAYS
#define PUBREL    0b01100000   // 6 || PUBLISH RELEASE  || BOTH WAYS
#define PUBCOMP   0b01110000   // 7 || PUBLISH COMPLETE || BOTH WAYS
//************

#define SUBSCRIBE 0b10000000   // 8 || SUBSCRIBE REQUEST || CLIENT TO SERVER
#define SUBACK    0b10010000   // 9 || SUBSCRIBE ACK     || SERVER TO CLIENT

// WILL NOT IMPLETEMT FOR NOW
#define UNSUBSCRIBE  0b10100000   // 10 || UNSUBSCRIBE REQUEST || CLIENT TO SERVER
#define UNSUBACK     0b10110000   // 11 || UNSUBSCRIBE ACK     || SERVER TO CLIENT
#define PINGREQ      0b11000000   // 12 || PING REQUEST  || CLIENT TO SERVER
#define PINGRESP     0b11010000   // 13 || PING RESPONSE || SERVER TO CLIENT
#define DISCONNECT   0b11100000   // 14 || CLIENT IS DISCONNECTING || BOTH WAYS
#define AUTH         0b11110000   // 15 || AUTENTICATION EXCHANGE  || BOTH WAYS
//************

/*******************************************/
/*                                         */
/*                  FLAGS                  */
/*                                         */
/*******************************************/

#define DUP    0b00000001 // DUPLICATE DELIVERY PUBLISH
#define QOS    0b00000110 // PUBLISH CUALITY OF SERVICE
#define RETAIN 0b00001000 // PUBLISH RETEINED MESSAGE FLAG

void handleFixedHeader(char *args);

//================================================================================================================

/*******************************************/
/*                                         */
/*         CONNECT VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct {
    // 4
    uint16_t nameLenght;
    // "MQTT"
    char name[4];
    // 4 for 3.1 | 5 for 5
    uint8_t version;
    // type of connection
    uint8_t connectFlags;
    // the time in seconds between each user mqtt packet transmition
    uint16_t keepAlive;
} connectVariableHeader;

#define SIZE_CONN_VAR sizeof(uint16_t)+(sizeof(char)*4)+sizeof(uint8_t)+sizeof(uint8_t)+sizeof(uint16_t)

/*******************************************/
/*                                         */
/*              CONNECT FLAGS              */
/*                                         */
/*******************************************/

// 1 for new session 0 for existing sessing if there are no previus sessions
#define CLEANSTART  0b00000010
// 1 if the client wants to send others a message of a unespected disconection
#define WILLFLAG    0b00000100
// 1 | 2 | 3 depending on the level of assuranse that the user wants if the will flag is set to 1
#define WILLQOS     0b00011000
// if 1 the server must return the message as a retainable message
#define WILLRETAIN  0b00100000
// if set to 1 the payload has the password
#define PASSWORD    0b01000000
// if set to 1 the payload has the username
#define USERNAME    0b10000000

/*******************************************/
/*                                         */
/*             CONNECT PAYLOAD             */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t clientIDSize;
    char* clientID;
    uint16_t willTopicSize;
    char* willTopic;
    uint16_t willMessageSize;
    char* willMessage;
    uint16_t userNameSize;
    char* userName;
    uint16_t passWordSize;
    char* passWord;
} connectPayload;

void handleConnect(char* args, int offset);
connectPayload readConnectPayload(char* args, int offset);
void freeConnectPayload(connectPayload* payload);

//================================================================================================================

/*******************************************/
/*                                         */
/*         CONNACK VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct {
    uint8_t flags;
    uint8_t returnCode;
} connackVariableHeader;

/*******************************************/
/*                                         */
/*           CONNECT RETURN CODE           */
/*                                         */
/*******************************************/

#define ACCEPTED                0b00000000
#define REFUSED_VERSION         0b00000001
#define REFUSED_IDENTIFIER      0b00000010
#define REFUSED_SERVER_DOWN     0b00000011
#define REFUSED_WRONG_USER_PASS 0b00000100
#define REFUSED_NOT_AUTHORIZED  0b00000101

//================================================================================================================

/*******************************************/
/*                                         */
/*         PUBLISH VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t topicSize;
    char* topic;
    uint16_t identifier;
} publishVariableHeader;


/*******************************************/
/*                                         */
/*             PUBLISH PAYLOAD             */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t payloadSize;
    char* data;
} publishPayload;

void handlePublish(char* args, int offset);

//================================================================================================================

/*******************************************/
/*                                         */
/*          PUBACK VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t identifier;
} pubackVariableHeader;

//================================================================================================================

/*******************************************/
/*                                         */
/*        SUBSCRIBE VARIABLE HEADER        */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t identifier;
} subscribeVariableHeader;

/*******************************************/
/*                                         */
/*            SUBSCRIBE PAYLOAD            */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t payloadSize;
    char* topic;
    uint8_t qos;
} subscribePayload;

void handleSubscribe(char* args, int offset);

//================================================================================================================

/*******************************************/
/*                                         */
/*         SUBACK VARIABLE HEADER          */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t identifier;
} subackbeVariableHeader;

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

typedef struct {
    int sockfd;
    int clientfd;
} SOCKET;

// create the socket file descriptor with the ip, port and queue size
int createSocket(char* port, char* ip, int queue);

// prints the info of the server socket and the client socket
int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage* their_addr, socklen_t addr_size);

int handleRecv(void *args);

int handleServer(void *args);

int close_server();