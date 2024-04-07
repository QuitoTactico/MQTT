#include "mqttServer.h"

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

typedef struct
{
    uint8_t messageType;
    uint16_t remainingLenght;
} fixedHeader;

/*******************************************/
/*                                         */
/*              CONTROL PACKET             */
/*                                         */
/*******************************************/

#define FIXED 0b11110000

#define CONNECT 0b00010000 // 1 || CONNECT     || CLIENT TO SERVER //
#define CONNACK 0b00100000 // 2 || CONNECT ACK || SERVER TO CLIENT
#define PUBLISH 0b00110000 // 3 || PUBLISH MESSAGE || BOTH WAYS
#define PUBACK 0b01000000  // 4 || PUBLISH ACK     || BOTH WAYS

// WILL NOT IMPLETEMT FOR NOW
#define PUBREC 0b01010000  // 5 || PUBLISH RECIEVE  || BOTH WAYS
#define PUBREL 0b01100000  // 6 || PUBLISH RELEASE  || BOTH WAYS
#define PUBCOMP 0b01110000 // 7 || PUBLISH COMPLETE || BOTH WAYS
//************

#define SUBSCRIBE 0b10000000 // 8 || SUBSCRIBE REQUEST || CLIENT TO SERVER
#define SUBACK 0b10010000    // 9 || SUBSCRIBE ACK     || SERVER TO CLIENT

// WILL NOT IMPLETEMT FOR NOW
#define UNSUBSCRIBE 0b10100000 // 10 || UNSUBSCRIBE REQUEST || CLIENT TO SERVER
#define UNSUBACK 0b10110000    // 11 || UNSUBSCRIBE ACK     || SERVER TO CLIENT
#define PINGREQ 0b11000000     // 12 || PING REQUEST  || CLIENT TO SERVER
#define PINGRESP 0b11010000    // 13 || PING RESPONSE || SERVER TO CLIENT
#define DISCONNECT 0b11100000  // 14 || CLIENT IS DISCONNECTING || BOTH WAYS
#define AUTH 0b11110000        // 15 || AUTENTICATION EXCHANGE  || BOTH WAYS
//************

/*******************************************/
/*                                         */
/*                  FLAGS                  */
/*                                         */
/*******************************************/

#define DUP 0b00000001    // DUPLICATE DELIVERY PUBLISH
#define QOS 0b00000110    // PUBLISH CUALITY OF SERVICE
#define RETAIN 0b00001000 // PUBLISH RETEINED MESSAGE FLAG

void handleFixedHeader(char *args);

//================================================================================================================

/*******************************************/
/*                                         */
/*         CONNECT VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct
{
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

/*******************************************/
/*                                         */
/*              CONNECT FLAGS              */
/*                                         */
/*******************************************/

// 1 for new session 0 for existing sessing if there are no previus sessions
#define CLEANSTART 0b00000010
// 1 if the client wants to send others a message of a unespected disconection
#define WILLFLAG 0b00000100
// 1 | 2 | 3 depending on the level of assuranse that the user wants if the will flag is set to 1
#define WILLQOS 0b00011000
// if 1 the server must return the message as a retainable message
#define WILLRETAIN 0b00100000
// if set to 1 the payload has the password
#define PASSWORD 0b01000000
// if set to 1 the payload has the username
#define USERNAME 0b10000000

/*******************************************/
/*                                         */
/*             CONNECT PAYLOAD             */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t clientIDSize;
    char *clientID;
    uint16_t willTopicSize;
    char *willTopic;
    uint16_t willMessageSize;
    char *willMessage;
    uint16_t userNameSize;
    char *userName;
    uint16_t passWordSize;
    char *passWord;
} connectPayload;

void handleConnect(char *args, int offset);
connectPayload readConnectPayload(char *args, int offset);
void freeConnectPayload(connectPayload *payload);

//================================================================================================================

/*******************************************/
/*                                         */
/*           CONNECT RETURN CODE           */
/*                                         */
/*******************************************/

#define ACCEPTED 0b00000000
#define REFUSED_VERSION 0b00000001
#define REFUSED_IDENTIFIER 0b00000010
#define REFUSED_SERVER_DOWN 0b00000011
#define REFUSED_WRONG_USER_PASS 0b00000100
#define REFUSED_NOT_AUTHORIZED 0b00000101

//================================================================================================================

/*******************************************/
/*                                         */
/*         PUBLISH VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t size;
    char *topic;
    uint16_t identifier;
} publishVariableHeader;

/*******************************************/
/*                                         */
/*             PUBLISH PAYLOAD             */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t size;
    char *data;
} publishPayload;

void handlePublish(char *args, int offset);

//================================================================================================================

/*******************************************/
/*                                         */
/*        SUBSCRIBE VARIABLE HEADER        */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t identifier;
} subscribeVariableHeader;

/*******************************************/
/*                                         */
/*            SUBSCRIBE PAYLOAD            */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t size;
    char *topic;
    uint8_t qos;
} subscribePayload;

void handleSubscribe(char *args, int offset);
void freeSubscribe(subscribePayload *sp, int amount);

//================================================================================================================

#define UTF_HANDLE(name, field, sizeField, args, offset)  \
    memcpy(&(name.sizeField), args + offset, 2);           \
    name.sizeField = ntohs(name.sizeField);                \
                                                           \
    offset += 2;                                           \
                                                           \
    if (name.sizeField != 0)                               \
    {                                                      \
        do                                                 \
        {                                                  \
            name.field = (char *)malloc(name.sizeField);   \
        } while (name.field == NULL);                      \
                                                           \
        memcpy(name.field, args + offset, name.sizeField); \
                                                           \
        offset += name.sizeField;                          \
    }

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

void handleFixedHeader(char *args)
{
    int offset = 0;
    fixedHeader header;

    memcpy(&header.messageType, args + offset, 1);
    offset += 1;
    memcpy(&header.remainingLenght, args + offset, 2);
    header.remainingLenght = ntohs(header.remainingLenght);
    offset += 2;

    switch (header.messageType & FIXED)
    {
    case CONNECT:
        printf("###############################\n");
        printf("####### user connecting #######\n");
        printf("###############################\n\n");

        handleConnect(args, offset);
        break;
    case PUBLISH:
        printf("###############################\n");
        printf("####### user publishing #######\n");
        printf("###############################\n\n");

        handlePublish(args, offset);
        break;
    case SUBSCRIBE:
        printf("################################\n");
        printf("####### user subscribing #######\n");
        printf("################################\n\n");

        handleSubscribe(args, offset);
        break;
    default:
        printf("############################\n");
        printf("####### wrong header #######\n");
        printf("############################\n\n");

        printf("information: %s", args);
        break;
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

void handleConnect(char *args, int offset)
{
    connectVariableHeader variable;

    memcpy(&variable.nameLenght, args + offset, 2);
    variable.nameLenght = ntohs(variable.nameLenght);
    offset += 2;
    memcpy(&variable.name, args + offset, 4);
    offset += 4;
    memcpy(&variable.version, args + offset, 1);
    offset += 1;
    memcpy(&variable.connectFlags, args + offset, 1);
    offset += 1;
    memcpy(&variable.keepAlive, args + offset, 2);
    variable.keepAlive = ntohs(variable.keepAlive);
    offset += 2;

    if ((variable.connectFlags & CLEANSTART) == CLEANSTART)
    {
        printf("clean start\n");
    }
    if ((variable.connectFlags & WILLFLAG) == WILLFLAG)
    {
        printf("will flags\n");
    }
    if ((variable.connectFlags & WILLQOS) == WILLQOS)
    {
        printf("will q0s\n");
    }
    if ((variable.connectFlags & WILLRETAIN) == WILLRETAIN)
    {
        printf("will retain\n");
    }
    if ((variable.connectFlags & PASSWORD) == PASSWORD)
    {
        printf("pass word\n");
    }
    if ((variable.connectFlags & USERNAME) == USERNAME)
    {
        printf("user name\n");
    }

    connectPayload payload = readConnectPayload(args, offset);

    if (payload.clientIDSize != 0)
    {
        printf("clientID size: %d\n", payload.clientIDSize);
        printf("clientID: %s\n", payload.clientID);
    }
    if (payload.willTopicSize != 0)
    {
        printf("will topic size: %d\n", payload.willTopicSize);
        printf("will topic: %s\n", payload.willTopic);
    }
    if (payload.willMessageSize != 0)
    {
        printf("will message size: %d\n", payload.willMessageSize);
        printf("will message: %s\n", payload.willMessage);
    }
    if (payload.userNameSize != 0)
    {
        printf("user name size: %d\n", payload.userNameSize);
        printf("user name: %s\n", payload.userName);
    }
    if (payload.passWordSize != 0)
    {
        printf("password size: %d\n", payload.passWordSize);
        printf("password: %s\n", payload.passWord);
    }

    freeConnectPayload(&payload);
}

connectPayload readConnectPayload(char *args, int offset)
{
    connectPayload payload;

    //========client id size========

    UTF_HANDLE(payload, clientID, clientIDSize, args, offset);

    //========will topic size========

    UTF_HANDLE(payload, willTopic, willTopicSize, args, offset);

    //========will message size========

    UTF_HANDLE(payload, willMessage, willMessageSize, args, offset);

    //========name size========

    UTF_HANDLE(payload, userName, userNameSize, args, offset);

    //========password size========

    UTF_HANDLE(payload, userName, userNameSize, args, offset);

    return payload;
}

void freeConnectPayload(connectPayload *payload)
{
    if (payload->clientIDSize != 0)
        free(payload->clientID);
    if (payload->willTopicSize != 0)
        free(payload->willTopic);
    if (payload->willMessageSize != 0)
        free(payload->willMessage);
    if (payload->userNameSize != 0)
        free(payload->userName);
    if (payload->passWordSize != 0)
        free(payload->passWord);
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 PUBLISH                 */
/*                                         */
/*******************************************/

void handlePublish(char *args, int offset)
{
    publishVariableHeader variable;

    UTF_HANDLE(variable, topic, size, args, offset);

    memcpy(&variable.identifier, args + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    offset += 2;

    free(variable.topic);

    publishPayload payload;

    UTF_HANDLE(payload, data, size, args, offset);

    free(payload.data);
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                SUBSCRIBE                */
/*                                         */
/*******************************************/

void handleSubscribe(char *args, int offset)
{
    subscribeVariableHeader variable;

    memcpy(&variable.identifier, args + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    offset += 2;

    subscribePayload payload[20];
    int amount_sub = 0;

    while((args + offset) != 0 || (args + offset + 1) != 0 && amount_sub < 20)
    {
        UTF_HANDLE(payload[amount_sub], topic, size, args, offset);

        memcpy(&payload[amount_sub].qos, args + offset, 1);

        offset += 1;

        amount_sub += 1;
    }

    freeSubscribe(payload, amount_sub);
}

void freeSubscribe(subscribePayload *sp, int amount)
{
    while (amount >= 0)
    {
        free(sp[amount].topic);
    }
    
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

mtx_t mutex;

int createSocket(char *ip, char *port, int queue)
{
    struct addrinfo hints, *res;

    // first, load up the address structs with getaddrinfo():

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if (getaddrinfo(ip, port, &hints, &res) != 0)
    {
        perror("GET ADDRES INFO");
        return -1;
    }

    // define the socket file descriptor to listen for incomming connections

    int sockfd;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd < 0)
    {
        perror("SOCKET");
        return -1;
    }

    // bind the socket file descriptor to the port

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0)
    {
        perror("BINDING");
        return -1;
    }

    // define the size of the queue to listen for incomming connections

    if (listen(sockfd, QUEUESIZE) != 0)
    {
        perror("LISTENING");
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;
}

int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage *their_addr, socklen_t addr_size)
{
    const int NAMESIZE = 100;
    char hostName[NAMESIZE];
    memset(&hostName, 0, NAMESIZE);
    char serviceName[NAMESIZE];
    memset(&serviceName, 0, NAMESIZE);
    int flags = NI_NUMERICHOST + NI_NUMERICSERV;

    if (getnameinfo((struct sockaddr *)their_addr, addr_size, hostName, NAMESIZE, serviceName, NAMESIZE, flags) == -1)
    {
        perror("GET NAME INFO");
        return -1;
    }

    printf("####### ip conection from %s with port %s #######\n\n", hostName, serviceName);
}

int handleRecv(void *args)
{
    int sockfd = *(int *)args;

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    int clientfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (clientfd == -1)
    {
        perror("ACCEPT");
        return 1;
    }

    printf("####### connection accepted #######\n\n");

    if (printSocketInfo(sockfd, clientfd, &their_addr, addr_size) < 0)
    {
        close(clientfd);
        return 1;
    }

    // print the info recieved by the client
    char buf[1000];
    int len = 1000;

    int num = 1;

    while (buf[0] != 'q')
    {
        recv(clientfd, buf, len, 0);

        printf("===== time %d =====\n\n", num);
        num++;
        // printf("info del cliente %d:\n\n%s\n", clientfd, buf);

        if (buf[0] == 'q')
            break;

        handleFixedHeader(buf);

        memset(buf, 0, len);
    }

    printf("####### client gone #######\n\n");

    memset(buf, 0, len);

    close(clientfd);

    return 0;
}

char quit[5];
int stop = 1;

int handleServer(void *args)
{
    while (1)
    {
        printf("Enter 'quit' to exit: ");
        fgets(quit, sizeof(quit), stdin);
        // Remove newline character if present
        quit[strcspn(quit, "\n")] = 0;

        if (strcmp(quit, "quit") == 0)
        {
            stop = 0;
            printf("Exiting...\n");
            break;
        }
    }

    return 0;
}

int close_server()
{
    return stop;
}