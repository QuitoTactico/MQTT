#include "mqttBroker.h"

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

typedef struct
{
    uint8_t messageType;
    uint32_t remainingLenght;
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

// WILL NOT IMPLEMENT FOR NOW
#define PUBREC 0b01010000  // 5 || PUBLISH RECEIVE  || BOTH WAYS
#define PUBREL 0b01100000  // 6 || PUBLISH RELEASE  || BOTH WAYS
#define PUBCOMP 0b01110000 // 7 || PUBLISH COMPLETE || BOTH WAYS
//************

#define SUBSCRIBE 0b10000000 // 8 || SUBSCRIBE REQUEST || CLIENT TO SERVER
#define SUBACK 0b10010000    // 9 || SUBSCRIBE ACK     || SERVER TO CLIENT

// WILL NOT IMPLEMENT FOR NOW
#define UNSUBSCRIBE 0b10100000 // 10 || UNSUBSCRIBE REQUEST     || CLIENT TO SERVER
#define UNSUBACK 0b10110000    // 11 || UNSUBSCRIBE ACK         || SERVER TO CLIENT
#define PINGREQ 0b11000000     // 12 || PING REQUEST            || CLIENT TO SERVER
#define PINGRESP 0b11010000    // 13 || PING RESPONSE           || SERVER TO CLIENT
#define DISCONNECT 0b11100000  // 14 || CLIENT IS DISCONNECTING || BOTH WAYS
#define AUTH 0b11110000        // 15 || AUTHENTICATION EXCHANGE || BOTH WAYS
//************

/*******************************************/
/*                                         */
/*                  FLAGS                  */
/*                                         */
/*******************************************/

#define DUP 0b00000001    // DUPLICATE DELIVERY PUBLISH
#define QOS 0b00000110    // PUBLISH QUALITY OF SERVICE
#define RETAIN 0b00001000 // PUBLISH RETAINED MESSAGE FLAG

void handleFixedHeader(char *args, int sockfd);

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
    // the time in seconds between each user mqtt packet transmission
    uint16_t keepAlive;
} connectVariableHeader;

/*******************************************/
/*                                         */
/*              CONNECT FLAGS              */
/*                                         */
/*******************************************/

// 1 for new session 0 for existing session if there are no previous sessions
#define CLEANSTART 0b00000010
// 1 if the client wants to send others a message of a unexpected disconection
#define WILLFLAG 0b00000100
// 1 | 2 | 3 depending on the level of assurance that the user wants if the will flag is set to 1
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

void handleConnect(char *args, int offset, int sockfd);
connectPayload readConnectPayload(char *args, int offset);
void freeConnectPayload(connectPayload *payload);
void sendConnack(int sockfd);

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

void handlePublish(char *args, int offset, int sockfd);
void sendPuback(int sockfd, uint16_t id);

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

void handleSubscribe(char *args, int offset, int sockfd);
void freeSubscribe(subscribePayload *sp, int amount);
void sendSuback(int sockfd, uint16_t id, subscribePayload *sp, int amount);

//================================================================================================================

uint32_t remainingOffset(uint32_t value)
{
    if (value <= 0xFF) {
        return 1;
    } else if (value <= 0xFFFF) {
        return 2;
    } else if (value <= 0xFFFFFF) {
        return 3;
    } else {
        return 4;
    }
}

uint32_t decodeRemainingLength(const char* buffer) {
    uint32_t value = 0;
    uint32_t multiplier = 1;
    size_t index = 0;

    do {
        uint8_t encodedByte = buffer[index++];
        value += (encodedByte & 127) * multiplier;

        multiplier *= 128;
        if (multiplier > 128*128*128) {
            fprintf(stderr, "remaining length malformado en el paquete MQTT\n");
            return 0;
        }
    } while ((buffer[index-1] & 128) != 0);

    return value;
}

// macro for the utf handling of inputs
#define UTF_HANDLE(name, field, sizeField, args, offset)        \
    memcpy(&(name.sizeField), args + offset, 2);                \
    name.sizeField = ntohs(name.sizeField);                     \
                                                                \
    offset += 2;                                                \
                                                                \
    if (name.sizeField != 0)                                    \
    {                                                           \
        do                                                      \
        {                                                       \
            name.field = (char *)malloc(name.sizeField + 1);    \
        } while (name.field == NULL);                           \
                                                                \
        memcpy(name.field, args + offset, name.sizeField);      \
                                                                \
        name.field[name.sizeField] = '\0';                      \
                                                                \
        offset += name.sizeField;                               \
    }

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

void handleFixedHeader(char *args, int sockfd)
{
    int offset = 0;
    fixedHeader header;

    memcpy(&header.messageType, args + offset, 1);
    offset += 1;
    
    header.remainingLenght = decodeRemainingLength(args + 1);

    offset += remainingOffset(header.remainingLenght);

    switch (header.messageType & FIXED)
    {
    case CONNECT:
        printf("###############################\n");
        printf("####### user connecting #######\n");
        printf("###############################\n\n");

        handleConnect(args, offset, sockfd);
        break;
    case PUBLISH:
        printf("###############################\n");
        printf("####### user publishing #######\n");
        printf("###############################\n\n");

        handlePublish(args, offset, sockfd);
        break;
    case SUBSCRIBE:
        printf("################################\n");
        printf("####### user subscribing #######\n");
        printf("################################\n\n");

        handleSubscribe(args, offset, sockfd);
        break;
    default:
        printf("############################\n");
        printf("####### wrong header #######\n");
        printf("############################\n\n");

        printf("information: %s", args);
        printf("received message: ");
        for (int i = 7; i >= 0; i--) {
            printf("%d", (header.messageType >> i) & 1);
        }
        printf("\n");
        break;
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

void handleConnect(char *args, int offset, int sockfd)
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

    connectPayload payload = readConnectPayload(args, offset);

    if ((variable.connectFlags & CLEANSTART) == CLEANSTART)
    {
        printf("-clean start-\n");
    }
    if ((variable.connectFlags & WILLFLAG) == WILLFLAG)
    {
        printf("-has will flags-\n");
    }
    if ((variable.connectFlags & WILLQOS) == WILLQOS)
    {
        printf("-has will qos-\n");
    }
    if ((variable.connectFlags & WILLRETAIN) == WILLRETAIN)
    {
        printf("-has will retain-\n");
    }
    if ((variable.connectFlags & PASSWORD) == PASSWORD)
    {
        printf("-has password-\n");
    }
    if ((variable.connectFlags & USERNAME) == USERNAME)
    {
        printf("-has username-\n");
    }

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

    sendConnack(sockfd);

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

    UTF_HANDLE(payload, passWord, passWordSize, args, offset);

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

void sendConnack(int sockfd)
{
    char connackMessage[4];

    connackMessage[0] = CONNACK;

    // remaining lenght
    connackMessage[1] = 0x02;

    // sesion present
    connackMessage[2] = 0x00;

    // state
    connackMessage[3] = ACCEPTED;

    int result = send(sockfd, connackMessage, 4, 0);
    
    if (result == -1) {
        perror("Sending connack failed\n");
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 PUBLISH                 */
/*                                         */
/*******************************************/

void handlePublish(char *args, int offset, int sockfd)
{
    // publish variable header

    publishVariableHeader variable;

    UTF_HANDLE(variable, topic, size, args, offset);

    memcpy(&variable.identifier, args + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    offset += 2;

    // publish payload

    publishPayload payload;

    UTF_HANDLE(payload, data, size, args, offset);

    // printing information

    if(variable.size !=0)
    {
        printf("publish topic size: %d\n", variable.size);
        printf("publish topic: %s\n", variable.topic);
    }
    if(payload.size !=0)
    {
        printf("publish data size: %d\n", payload.size);
        printf("publish data: %s\n", payload.data);
    }

    sendPuback(sockfd, variable.identifier);

    free(variable.topic);
    free(payload.data);
}

void sendPuback(int sockfd, uint16_t id)
{
    char pubackMessage[4];

    pubackMessage[0] = PUBACK;

    // remaining lenght
    pubackMessage[1] = 0x02;

    id = htons(id);
    memcpy(pubackMessage + 2, &id, 2);

    int result = send(sockfd, pubackMessage, 4, 0);
    
    if (result == -1) {
        perror("Sending connack failed\n");
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                SUBSCRIBE                */
/*                                         */
/*******************************************/

void handleSubscribe(char *args, int offset, int sockfd)
{
    subscribeVariableHeader variable;

    memcpy(&variable.identifier, args + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    offset += 2;

    subscribePayload payload[200];
    int amount_sub = 0;

    while(((args[offset] != 0) || (args[offset + 1] != 0)) && amount_sub < 200)
    {
        UTF_HANDLE(payload[amount_sub], topic, size, args, offset);

        memcpy(&payload[amount_sub].qos, args + offset, 1);

        offset += 1;

        amount_sub += 1;
    }

    printf("subscribe id: %d\n", variable.identifier);

    for (int i = 0; i < amount_sub; i++)
    {
        printf("subscribe topic size: %d\n", payload[i].size);
        printf("subscribe topic: %s\n", payload[i].topic);
        printf("subscribe topic qos: %d\n", payload[i].qos);
    }

    freeSubscribe(payload, amount_sub);
}

void freeSubscribe(subscribePayload *sp, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        if (sp[amount].topic != NULL)
            free(sp[amount].topic);
    }
    
}

void sendSuback(int sockfd, uint16_t id, subscribePayload *sp, int amount)
{
    int offset = 2;

    char pubackMessage[2 + amount];

    pubackMessage[0] = SUBACK;

    // remaining lenght
    pubackMessage[1] = amount;

    // identifier
    id = ntohs(id);
    memcpy(pubackMessage + 2, &id, 2);

    for (int i = 0; i < amount; i++)
    {
        memcpy(pubackMessage + offset, &sp->qos, 1);
        offset++;
    }

    int result = send(sockfd, pubackMessage, amount + 3, 0);
    
    if (result == -1) {
        perror("Sending connack failed\n");
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
    int brokersockfd = *(int *)args;

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    int clientsockfd = accept(brokersockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (clientsockfd == -1)
    {
        perror("ACCEPT");
        return 1;
    }

    printf("####### connection accepted #######\n\n");

    if (printSocketInfo(brokersockfd, clientsockfd, &their_addr, addr_size) < 0)
    {
        close(clientsockfd);
        return 1;
    }

    // print the info recieved by the client
    char buf[1000];
    int len = 1000;

    int num = 1;

    while (buf[0] != 'q')
    {
        recv(clientsockfd, buf, len, 0);

        printf("===== time %d =====\n\n", num);
        num++;
        // printf("info del cliente %d:\n\n%s\n", clientfd, buf);

        if (buf[0] == 'q')
            break;

        handleFixedHeader(buf, clientsockfd);

        memset(buf, 0, len);
    }

    printf("####### client gone #######\n\n");

    memset(buf, 0, len);

    close(clientsockfd);

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