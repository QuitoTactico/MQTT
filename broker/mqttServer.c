#include "mqttServer.h"

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

    memcpy(&header.messageType, args + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&header.remainingLenght, args + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    switch (header.messageType & FIXED)
    {
        case CONNECT:
            printf("####### user connecting #######\n\n");
            handleConnect(args, offset);
            break;
        case PUBLISH:
            handlePublish(args, offset);
            break;
        case SUBSCRIBE:
            handleSubscribe(args, offset);
            break;
        default:
            printf("information: %s", args);
            printf("####### wrong header #######\n\n");
            break;
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

void handleConnect(char* args, int offset)
{
    connectVariableHeader variable;

    memcpy(&variable.nameLenght, args + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&variable.name, args + offset, sizeof(char) * 4);
    offset += sizeof(char) * 4;
    memcpy(&variable.version, args + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&variable.connectFlags, args + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&variable.keepAlive, args + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    if ((variable.connectFlags & CLEANSTART) == CLEANSTART)
    {

    }
    if ((variable.connectFlags & WILLFLAG) == WILLFLAG)
    {

    }
    if ((variable.connectFlags & WILLQOS) == WILLQOS)
    {

    }
    if ((variable.connectFlags & WILLRETAIN) == WILLRETAIN)
    {

    }
    if ((variable.connectFlags & PASSWORD) == PASSWORD)
    {

    }
    if ((variable.connectFlags & USERNAME) == USERNAME)
    {

    }

    connectPayload payload = readConnectPayload(args, offset);

    printf("SIUUUUUUUUUU: %s\n\n", payload.clientID);

    freeConnectPayload(&payload);

    memset(&payload, 0, sizeof(connectPayload));
}

connectPayload readConnectPayload(char* args, int offset)
{
    connectPayload payload;

    //========client id size========

    memcpy(&payload.clientIDSize, args + offset, sizeof(uint16_t));

    offset += sizeof(uint16_t);

    //========client id========

    if (payload.clientIDSize != 0)
    {
        do {
            payload.clientID = (char *)malloc(payload.clientIDSize);
        } while (payload.clientID == NULL);
        

        memcpy(payload.clientID, args + offset, payload.clientIDSize);
    }

    offset += payload.clientIDSize;

    //========will topic size========

    memcpy(&payload.willTopicSize, args + offset, sizeof(uint16_t));

    offset += sizeof(uint16_t);

    //========will topic========

    if (payload.willTopicSize != 0)
    {
        do {
            payload.willTopic = (char *)malloc(payload.willTopicSize);
        } while (payload.willTopic == NULL);

        memcpy(payload.willTopic, args + offset, payload.willTopicSize);
    }
    
    offset += payload.willTopicSize;

    //========will message size========

    memcpy(&payload.willMessageSize, args + offset, sizeof(uint16_t));

    offset += sizeof(uint16_t);

    //========will message========

    if (payload.willMessageSize != 0)
    {
        do {
            payload.willMessage = (char *)malloc(payload.willMessageSize);
        } while (payload.willMessage == NULL);

        memcpy(payload.willMessage, args + offset, payload.willMessageSize);
    }

    offset += payload.willMessageSize;

    //========name size========

    memcpy(&payload.userNameSize, args + offset, sizeof(uint16_t));

    offset += sizeof(uint16_t);

    //========name========

    if (payload.userNameSize != 0)
    {
        do {
            payload.userName = (char *)malloc(payload.userNameSize);
        } while (payload.userName == NULL);

        memcpy(payload.userName, args + offset, payload.userNameSize);
    }
    
    offset += payload.userNameSize;

    //========password size========

    memcpy(&payload.passWordSize, args + offset, sizeof(uint16_t));

    offset += sizeof(uint16_t);

    //========password========

    if (payload.passWordSize != 0)
    {
        do {
            payload.passWord = (char *)malloc(payload.passWordSize);
        } while (payload.passWord == NULL);

        memcpy(payload.passWord, args + offset, payload.passWordSize);
    }
    
    offset += payload.passWordSize;

    return payload;
}

void freeConnectPayload(connectPayload* payload)
{
    if (payload->clientIDSize != 0) free(payload->clientID);
    if (payload->willTopicSize != 0) free(payload->willTopic);
    if (payload->willMessageSize != 0) free(payload->willMessage);
    if (payload->userNameSize != 0) free(payload->userName);
    if (payload->passWordSize != 0) free(payload->passWord);
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 PUBLISH                 */
/*                                         */
/*******************************************/

void handlePublish(char* args, int offset)
{

}

//================================================================================================================

/*******************************************/
/*                                         */
/*            SUBSCRIBE PAYLOAD            */
/*                                         */
/*******************************************/

void handleSubscribe(char* args, int offset)
{

}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

mtx_t mutex;

int createSocket(char* ip, char* port, int queue)
{
    struct addrinfo hints, *res;

    // first, load up the address structs with getaddrinfo():

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4 
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

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

int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage* their_addr, socklen_t addr_size)
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
        close(sockfd);
        close(clientfd);
        return -1;
    }

    printf("####### ip conection from %s with port %s #######\n", hostName, serviceName);
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

    if(printSocketInfo(sockfd, clientfd, &their_addr, addr_size) < 0)
    {
        close(clientfd);
        return 1;
    }

    // print the info recieved by the client 
    char buf[1000];
    int len = 1000; 

    int num = 1;

    while(buf[0] != 'q')
    {
        recv(clientfd, buf, len, 0);

        printf("===== time %d =====\n\n", num);
        num++;
        //printf("info del cliente %d:\n\n%s\n", clientfd, buf);
        
        if(buf[0] == 'q') break;

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

int handleServer(void* args)
{
    while (1) {
        printf("Enter 'quit' to exit: ");
        fgets(quit, sizeof(quit), stdin);
        // Remove newline character if present
        quit[strcspn(quit, "\n")] = 0;

        // Convert quit to lowercase for case-insensitive comparison
        for (int i = 0; quit[i]; i++) {
            quit[i] = tolower(quit[i]);
        }

        if (strcmp(quit, "quit") == 0) {
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