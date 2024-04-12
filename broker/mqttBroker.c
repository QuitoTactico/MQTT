#include "mqttBroker.h"

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

typedef struct fixedHeader
{
    uint8_t messageType;
    uint32_t remainingLenght;
} fixedHeader;

/**********************/
/*                    */
/*   CONTROL PACKET   */
/*                    */
/**********************/

#define FIXED 0b11110000

#define CONNECT 0b00010000 // 1 || CONNECT     || CLIENT TO SERVER //
#define CONNACK 0b00100000 // 2 || CONNECT ACK || SERVER TO CLIENT
#define PUBLISH 0b00110000 // 3 || PUBLISH MESSAGE || BOTH WAYS
#define PUBACK 0b01000000  // 4 || PUBLISH ACK     || BOTH WAYS

/* WILL NOT IMPLEMENT FOR NOW
#define PUBREC 0b01010000  // 5 || PUBLISH RECEIVE  || BOTH WAYS
#define PUBREL 0b01100000  // 6 || PUBLISH RELEASE  || BOTH WAYS
#define PUBCOMP 0b01110000 // 7 || PUBLISH COMPLETE || BOTH WAYS
*/

#define SUBSCRIBE 0b10000000 // 8 || SUBSCRIBE REQUEST || CLIENT TO SERVER
#define SUBACK 0b10010000    // 9 || SUBSCRIBE ACK     || SERVER TO CLIENT

/* WILL NOT IMPLEMENT FOR NOW
#define UNSUBSCRIBE 0b10100000 // 10 || UNSUBSCRIBE REQUEST     || CLIENT TO SERVER
#define UNSUBACK 0b10110000    // 11 || UNSUBSCRIBE ACK         || SERVER TO CLIENT
#define PINGREQ 0b11000000     // 12 || PING REQUEST            || CLIENT TO SERVER
#define PINGRESP 0b11010000    // 13 || PING RESPONSE           || SERVER TO CLIENT
#define DISCONNECT 0b11100000  // 14 || CLIENT IS DISCONNECTING || BOTH WAYS
#define AUTH 0b11110000        // 15 || AUTHENTICATION EXCHANGE || BOTH WAYS
*/

/**********************/
/*                    */
/*    FIXED FLAGS     */
/*                    */
/**********************/

#define DUP 0b00000001    // DUPLICATE DELIVERY PUBLISH
#define QOS 0b00000110    // PUBLISH QUALITY OF SERVICE
#define RETAIN 0b00001000 // PUBLISH RETAINED MESSAGE FLAG

// ---------------------------------------
// defined in auxiliar functions
uint32_t decodeRemainingLength(const char* buffer);

fixedHeader handleFixedHeader(char *message, int sockfd)
{
    fixedHeader header;

    memcpy(&header.messageType, message, 1);
    
    header.remainingLenght = decodeRemainingLength(message + 1);
    return header;
}



//================================================================================================================



/*******************************************/
/*                                         */
/*            AUXILIAR FUNCTIONS           */
/*                                         */
/*******************************************/

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

void getSocketIP(int sockfd, char* ip) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sockfd, (struct sockaddr *)&addr, &addr_size);

    if (res != 0) {
        printf("Error obteniendo la dirección IP\n");
        return;
    }

    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
}

// macro for the utf handling of inputs
#define UTF_HANDLE(name, field, sizeField, message, offset)        \
    memcpy(&(name.sizeField), message + offset, 2);                \
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
        memcpy(name.field, message + offset, name.sizeField);      \
                                                                \
        name.field[name.sizeField] = '\0';                      \
                                                                \
        offset += name.sizeField;                               \
    }



//================================================================================================================



/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

/***************************/
/*                         */
/* CONNECT VARIABLE HEADER */
/*                         */
/***************************/

typedef struct connectVariableHeader
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

/***************************/
/*                         */
/*      CONNECT FLAGS      */
/*                         */
/***************************/

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

/***************************/
/*                         */
/*     CONNECT PAYLOAD     */
/*                         */
/***************************/

typedef struct connectPayload
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

/***************************/
/*                         */
/*      CONNACK CODES      */
/*                         */
/***************************/

#define ACCEPTED 0b00000000
#define REFUSED_VERSION 0b00000001
#define REFUSED_IDENTIFIER 0b00000010
#define REFUSED_SERVER_DOWN 0b00000011
#define REFUSED_WRONG_USER_PASS 0b00000100
#define REFUSED_NOT_AUTHORIZED 0b00000101

// ---------------------------------------

connectVariableHeader readConnectVariableHeader(char *message, int offset)
{
    connectVariableHeader variable;

    memcpy(&variable.nameLenght, message + offset, 2);
    variable.nameLenght = ntohs(variable.nameLenght);
    offset += 2;
    memcpy(&variable.name, message + offset, 4);
    offset += 4;
    memcpy(&variable.version, message + offset, 1);
    offset += 1;
    memcpy(&variable.connectFlags, message + offset, 1);
    offset += 1;
    memcpy(&variable.keepAlive, message + offset, 2);
    variable.keepAlive = ntohs(variable.keepAlive);
    offset += 2;

    return variable;
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

connectPayload readConnectPayload(char *message, int offset)
{
    connectPayload payload;

    //========client id size========

    UTF_HANDLE(payload, clientID, clientIDSize, message, offset);

    //========will topic size========

    UTF_HANDLE(payload, willTopic, willTopicSize, message, offset);

    //========will message size========

    UTF_HANDLE(payload, willMessage, willMessageSize, message, offset);

    //========name size========

    UTF_HANDLE(payload, userName, userNameSize, message, offset);

    //========password size========

    UTF_HANDLE(payload, passWord, passWordSize, message, offset);

    return payload;
}

void sendConnack(int sockfd, connectVariableHeader variable, connectPayload payload)
{
    char connackMessage[4];

    connackMessage[0] = CONNACK;

    // remaining lenght
    connackMessage[1] = 0x02;

    // sesion present
    connackMessage[2] = 0x00;

    // state
    if(variable.version != 0x05){
        connackMessage[3] = REFUSED_VERSION;
    }
    else if((payload.clientIDSize == 0) || (payload.clientIDSize > 23)){
        connackMessage[3] = REFUSED_IDENTIFIER;
    }
    else if(DBverifySession(payload.userName, payload.passWord) != 1){
        connackMessage[3] = REFUSED_WRONG_USER_PASS;
    }
    else{
        connackMessage[3] = ACCEPTED;
    }
    

    int result = send(sockfd, connackMessage, 4, 0);
    
    if (result == -1) {
        perror("Sending connack failed\n");
    }
}

// ---------------------------------------

void handleConnect(char *message, int offset, int sockfd)
{
    connectVariableHeader variable = readConnectVariableHeader(message, offset);
    offset += 10;

    connectPayload payload = readConnectPayload(message, offset);

    if ((variable.connectFlags & CLEANSESSION) == CLEANSESSION)
    {
        printf("-clean start-\n");
        // si el cliente ingresó usuario y contraseña, se creará esa sesión
        if (((variable.connectFlags & USERNAME) == USERNAME) && ((variable.connectFlags & PASSWORD) == PASSWORD)){
            int createdOrUpdated = DBupdateOrCreate("dbSessions.csv", payload.userName, payload.passWord);
            if(createdOrUpdated == -1){
                printf("Session with username and password CREATED\n");
            }else if(createdOrUpdated == 1){
                printf("Session with username and password UPDATED\n");
            } 
        // si no los ingresó, o sólo ingresó uno, su usuario y contraseña serán su ID
        }else{
            DBupdateOrCreate("dbSessions.csv", payload.userName, payload.passWord);
            printf("Session with username and password equals to the ID. Username: %s, Password: %s\n", payload.clientID, payload.clientID);
        }
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

    sendConnack(sockfd, variable, payload);

    freeConnectPayload(&payload);
}



//================================================================================================================



/*******************************************/
/*                                         */
/*                 PUBLISH                 */
/*                                         */
/*******************************************/

/***************************/
/*                         */
/* PUBLISH VARIABLE HEADER */
/*                         */
/***************************/

typedef struct publishVariableHeader
{
    uint16_t size;
    char *topic;
    uint16_t identifier;
} publishVariableHeader;

/***************************/
/*                         */
/*     PUBLISH PAYLOAD     */
/*                         */
/***************************/

typedef struct publishPayload
{
    uint16_t size;
    char *data;
} publishPayload;

// ---------------------------------------

publishVariableHeader readPublishVariableHeader(char *message, int offset)
{
    publishVariableHeader variable;

    UTF_HANDLE(variable, topic, size, message, offset);

    memcpy(&variable.identifier, message + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    return variable;
}

publishPayload readPublishPayload(char *message, int offset)
{
    publishPayload payload;

    UTF_HANDLE(payload, data, size, message, offset);

    return payload;
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
        perror("Sending puback failed\n");
    }
}

// ---------------------------------------

void handlePublish(char *message, int offset, int sockfd)
{
    
    publishVariableHeader variable = readPublishVariableHeader(message, offset);
    offset += 2 + variable.size + 2;

    publishPayload payload = readPublishPayload(message, offset);

    // printing received information
    if(variable.size !=0)
    {
        printf("publish topic size: %d\n", variable.size);
        printf("publish topic: %s\n", variable.topic);
    }
    if (variable.identifier != 0){
        printf("publish identifier: %d\n", variable.identifier);
    }
    if(payload.size !=0)
    {
        printf("publish data size: %d\n", payload.size);
        printf("publish data: %s\n", payload.data);
    }

    // creating or updating the topic and its data in the database
    int res = DBupdateOrCreate("dbTopics.csv", variable.topic, payload.data);
    if (res != 0)
    {
        printf("Publicado correctamente\n");
    }
    else
    {
        printf("No se pudo publicar\n");
    }

    // sending the puback
    sendPuback(sockfd, variable.identifier);

    // there are so few utf-8 arguments in both that it was better to do the memory freeing here
    free(variable.topic);
    free(payload.data);
}



//================================================================================================================



/*******************************************/
/*                                         */
/*                SUBSCRIBE                */
/*                                         */
/*******************************************/

/***************************/
/*                         */
/*SUBSCRIBE VARIABLE HEADER*/
/*                         */
/***************************/

typedef struct subscribeVariableHeader
{
    uint16_t identifier;
}subscribeVariableHeader;

/***************************/
/*                         */
/*    SUBSCRIBE PAYLOAD    */
/*                         */
/***************************/

typedef struct subscribePayload
{
    uint16_t size;
    char *topic;
    uint8_t qos;
} subscribePayload;

// ---------------------------------------

subscribeVariableHeader readSubscribeVariableHeader(char *message, int offset)
{
    subscribeVariableHeader variable;

    memcpy(&variable.identifier, message + offset, 2);
    variable.identifier = ntohs(variable.identifier);

    return variable;
}

void freeSubscribePayload(subscribePayload *payload, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        if (payload[amount].topic != NULL)
            free(payload[amount].topic);
    }
    
}

void sendSuback(int sockfd, uint16_t id, subscribePayload *payload, int amount)
{
    int offset = 2;

    char subackMessage[2 + amount];

    subackMessage[0] = SUBACK;

    // remaining lenght
    subackMessage[1] = amount;

    // identifier
    id = ntohs(id);
    memcpy(subackMessage + 2, &id, 2);

    for (int i = 0; i < amount; i++)
    {
        memcpy(subackMessage + offset, &payload->qos, 1);
        offset++;
    }

    int result = send(sockfd, subackMessage, amount + 3, 0);
    
    if (result == -1) {
        perror("Sending SUBACK failed\n");
    }   
}

// ---------------------------------------

void handleSubscribe(char *message, int offset, int sockfd)
{
    subscribeVariableHeader variable = readSubscribeVariableHeader(message, offset);
    offset += 2;

    subscribePayload payload[200];
    int amount = 0;

    while(((message[offset] != 0) || (message[offset + 1] != 0)) && amount < 200)
    {
        UTF_HANDLE(payload[amount], topic, size, message, offset);

        memcpy(&payload[amount].qos, message + offset, 1);

        offset += 1;

        amount += 1;
    }

    printf("subscribe id: %d\n", variable.identifier);

    for (int i = 0; i < amount; i++)
    {
        printf("subscribe topic size: %d\n", payload[i].size);
        printf("subscribe topic: %s\n", payload[i].topic);
        printf("subscribe topic qos: %d\n", payload[i].qos);

        DBupdateOrCreate("dbSubscribes.csv", variable.identifier, payload[i].topic);
    }

    sendSuback(sockfd, variable.identifier, payload, amount);

    freeSubscribePayload(payload, amount);
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



//================================================================================================================



/*******************************************/
/*                                         */
/*                  MAIN                   */
/*                                         */
/*******************************************/

char quit[5];
int stop = 1;

int close_server()
{
    return stop;
}

// This is basically the SECOND MAIN. It is the function that is called in 
// the broker.c to handle server commands, like "quit".
int handleServer(void *message)
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

// ---------------------------------------

int handleMessage(char *message, int sockfd){

    fixedHeader header = handleFixedHeader(message, sockfd);

    int offset = 1 + remainingOffset(header.remainingLenght);
    uint8_t qos = (header.messageType&QOS) >> 1;

    switch (header.messageType & FIXED)
    {
    case CONNECT:
        printf("###############################\n");
        printf("####### user connecting #######\n");
        printf("###############################\n\n");

        handleConnect(message, offset, sockfd);
        break;
    case PUBLISH:
        printf("###############################\n");
        printf("####### user publishing #######\n");
        printf("###############################\n\n");

        handlePublish(message, offset, sockfd);
        break;
    case SUBSCRIBE:
        printf("################################\n");
        printf("####### user subscribing #######\n");
        printf("################################\n\n");

        handleSubscribe(message, offset, sockfd);
        break;
    default:
        printf("############################\n");
        printf("####### wrong header #######\n");
        printf("############################\n\n");

        printf("information: %s", message);
        printf("received message: ");
        for (int i = 7; i >= 0; i--) {
            printf("%d", (header.messageType >> i) & 1);
        }
        printf("\n");
        break;
    }
}

// This is like the THIRD MAIN function that is called in the broker.c to 
// handle the incoming messages. It first reads the fixed header to categorize
int handleRecv(void *message)
{
    int brokersockfd = *(int *)message;

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

        handleMessage(buf, clientsockfd);

        memset(buf, 0, len);
    }

    printf("####### client gone #######\n\n");

    memset(buf, 0, len);

    close(clientsockfd);

    return 0;
}
