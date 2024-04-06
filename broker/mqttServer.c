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
    if (payload.willMessage != 0)
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

    memcpy(&payload.clientIDSize, args + offset, 2);
    payload.clientIDSize = ntohs(payload.clientIDSize);

    offset += 2;

    //========client id========

    if (payload.clientIDSize != 0)
    {
        do
        {
            payload.clientID = (char *)malloc(payload.clientIDSize);
        } while (payload.clientID == NULL);

        memcpy(payload.clientID, args + offset, payload.clientIDSize);

        offset += payload.clientIDSize;
    }

    //========will topic size========

    memcpy(&payload.willTopicSize, args + offset, 2);
    payload.willTopicSize = ntohs(payload.willTopicSize);

    offset += 2;

    //========will topic========

    if (payload.willTopicSize != 0)
    {
        do
        {
            payload.willTopic = (char *)malloc(payload.willTopicSize);
        } while (payload.willTopic == NULL);

        memcpy(payload.willTopic, args + offset, payload.willTopicSize);

        offset += payload.willTopicSize;
    }

    //========will message size========

    memcpy(&payload.willMessageSize, args + offset, 2);
    payload.willMessageSize = ntohs(payload.willMessageSize);

    offset += 2;

    //========will message========

    if (payload.willMessageSize != 0)
    {
        do
        {
            payload.willMessage = (char *)malloc(payload.willMessageSize);
        } while (payload.willMessage == NULL);

        memcpy(payload.willMessage, args + offset, payload.willMessageSize);

        offset += payload.willMessageSize;
    }

    //========name size========

    memcpy(&payload.userNameSize, args + offset, 2);
    payload.userNameSize = ntohs(payload.userNameSize);

    offset += 2;

    //========name========

    if (payload.userNameSize != 0)
    {
        do
        {
            payload.userName = (char *)malloc(payload.userNameSize);
        } while (payload.userName == NULL);

        memcpy(payload.userName, args + offset, payload.userNameSize);

        offset += payload.userNameSize;
    }

    //========password size========

    memcpy(&payload.passWordSize, args + offset, 2);
    payload.passWordSize = ntohs(payload.passWordSize);

    offset += 2;

    //========password========

    if (payload.passWordSize != 0)
    {
        do
        {
            payload.passWord = (char *)malloc(payload.passWordSize);
        } while (payload.passWord == NULL);

        memcpy(payload.passWord, args + offset, payload.passWordSize);

        offset += payload.passWordSize;
    }

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
}

//================================================================================================================

/*******************************************/
/*                                         */
/*            SUBSCRIBE PAYLOAD            */
/*                                         */
/*******************************************/

void handleSubscribe(char *args, int offset)
{
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

        // Convert quit to lowercase for case-insensitive comparison
        for (int i = 0; quit[i]; i++)
        {
            quit[i] = tolower(quit[i]);
        }

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