#include "mqttClient.h"

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


//================================================================================================================

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
/*         SUBACK VARIABLE HEADER          */
/*                                         */
/*******************************************/

typedef struct {
    uint16_t identifier;
} subackbeVariableHeader;

//================================================================================================================

void utfHandle(char *message, char *type, int *offset)
{
    size_t longitud = 0;
    ssize_t readinfo;
    char *info;

    printf("%s", type);
    readinfo = getline(&info, &longitud, stdin);

    if (readinfo != -1)
    {
        int16_t len_info;
        int lenInfo = strlen(info) - 1;

        printf("You entered: %s", info);
        printf("The length of the message is: %u\n", lenInfo);

        info[strcspn(info, "\n")] = 0;

        len_info = htons(lenInfo);
        memcpy(message + *offset, &len_info, 2);
        *offset += 2;

        memcpy(message + *offset, info, lenInfo);
        *offset += lenInfo;
        free(info);
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

void createConnect(char *message)
{
    int offset = 0;

    // FIXED HEADER
    message[0] = CONNECT;

    offset += 1;

    uint16_t rem_lengt = htons(120);
    memcpy(message + offset, &rem_lengt, 2);

    offset += 2;

    // VARIABLE HEADER
    uint16_t title_size = htons(4);
    memcpy(message + offset, &title_size, 2);

    offset += 2;

    message[5] = 'M';
    message[6] = 'Q';
    message[7] = 'T';
    message[8] = 'T';

    offset += 4;

    // VERSION 3.1.1
    message[9] = 4;

    offset += 1;

    // FLAGS
    message[10] = 0;

    offset += 1;

    // KEEP ALIVE
    uint16_t keep_alive = htons(60);
    memcpy(message + offset, &keep_alive, 2);

    offset += 2;

    char asnwer[50];

    printf("Do you have a created session (0 no | 1 yes): ");
    scanf("%s", asnwer);
    getchar();

    if (strcmp(asnwer, "0") == 0)
    {
        utfHandle(message, "ID: ", &offset);

        char answerWill[20];
        printf("Do you want to leave the will flag (0 no | 1 yes): ");
        scanf("%s", answerWill);
        getchar();

        if (strcmp(answerWill, "1") == 0)
        {
            utfHandle(message, "will topic: ", &offset);

            utfHandle(message, "will message: ", &offset);
        }
        else
        {
            uint8_t i = 0;
            memcpy(message + offset, &i, 1);
            memcpy(message + offset + 1, &i, 1);
            memcpy(message + offset + 2, &i, 1);
            memcpy(message + offset + 3, &i, 1);
            offset += 4;
        }

        utfHandle(message, "user name: ", &offset);

        utfHandle(message, "password: ", &offset);

        for (size_t i = 0; i < offset; i++)
        {
            printf("%02X ", (unsigned char)message[i]); // Cast char to unsigned char for correct output
        }
        printf("\n\n");
    }
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                PUBLISH                  */
/*                                         */
/*******************************************/

void createPublish(char *message)
{

}

//================================================================================================================

/*******************************************/
/*                                         */
/*               SUBSCRIBE                 */
/*                                         */
/*******************************************/

void createSubscribe(char *message)
{

}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

int connectSocket(char *ip, char *port)
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

    connect(sockfd, res->ai_addr, res->ai_addrlen);

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

    printf("ip conection from %s with port %s\n", hostName, serviceName);
}