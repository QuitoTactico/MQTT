#include "mqttClient.h"

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
#define QOS 0b00000110    // PUBLISH CUALITY OF SERVICE 3 is reserved
#define QOS0 0b00000000   // QOS 0
#define QOS1 0b00000010   // QOS 1
#define QOS2 0b00000100   // QOS 2
#define RETAIN 0b00001000 // PUBLISH RETEINED MESSAGE FLAG

int handleFixHeader(char *message, uint8_t type);

//================================================================================================================

/*******************************************/
/*                                         */
/*              CONNECT FLAGS              */
/*                                         */
/*******************************************/

// 1 for new session 0 for existing session if there are no previous sessions
#define CLEANSESSION 0b00000010
// 1 if the client wants to send others a message of an unexpected disconnection
#define WILLFLAG 0b00000100
// 1 | 2 | 3 depending on the level of assurance that the user wants if the will flag is set to 1
#define WILLQOS1 0b00001000
#define WILLQOS2 0b00010000
// if 1 the server must return the message as a retainable message
#define WILLRETAIN 0b00100000
// if set to 1 the payload has the password
#define PASSWORD 0b01000000
// if set to 1 the payload has the username
#define USERNAME 0b10000000

//================================================================================================================

/*******************************************/
/*                                         */
/*         CONNACK VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct
{
    uint8_t flags;
    uint8_t returnCode;
} connackVariableHeader;

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

void handlePublish(char *args);

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

//================================================================================================================

/*******************************************/
/*                                         */
/*          PUBACK VARIABLE HEADER         */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t identifier;
} pubackVariableHeader;

//================================================================================================================

/*******************************************/
/*                                         */
/*         SUBACK VARIABLE HEADER          */
/*                                         */
/*******************************************/

typedef struct
{
    uint16_t identifier;
} subackbeVariableHeader;

//================================================================================================================
int counter;


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
    else
    {
        printf("Error reading line\n");
    }
}

int encodeRemainingLength(int length, char *output) {
    int digit;
    int pos = 0;

    do {
        digit = length % 128;
        length = length / 128;
        // Si hay más dígitos, establece el bit más significativo de este dígito
        if (length > 0) {
            digit = digit | 128;
        }
        output[pos++] = digit;
    } while (length > 0);

    return pos;
}

//================================================================================================================

/*******************************************/
/*                                         */
/*               FIXED HEADER              */
/*                                         */
/*******************************************/

int handleFixHeader(char *message, uint8_t type)
{
    message[0] = type;

    int qos;
    printf("Do you want a QoS (0 no | 1 yes): ");
    scanf("%d", &qos);
    getchar();

    if (qos) 
        //message |= QOS1;   anteriormente
        message[0] |= QOS1;

    int retain;
    printf("Do you want a retain (0 no | 1 yes): ");
    scanf("%d", &retain);
    getchar();

    if (retain)
        //message |= RETAIN;  anteriormente
        message[0] |= RETAIN;
    return qos;
}

//================================================================================================================

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

int createConnect(char *message)
{
    char variableAndPayload[500];
    
    int answerQoS;
    // FIXED HEADER
    handleFixHeader(message, CONNECT);

    int offset = 0;

    // VARIABLE HEADER
    uint16_t title_size = htons(4);
    memcpy(variableAndPayload + offset, &title_size, 2);

    offset += 2;

    variableAndPayload[offset++] = 'M';
    variableAndPayload[offset++] = 'Q';
    variableAndPayload[offset++] = 'T';
    variableAndPayload[offset++] = 'T';

    // VERSION 3.1.1
    variableAndPayload[offset++] = 4;

    // FLAGS
    int flags = offset;
    variableAndPayload[flags] = 0; // Initialize flags to 0

    int answerWill;
    printf("Do you want a Will (0 no | 1 yes): ");
    scanf("%d", &answerWill);
    getchar();

    if (answerWill)
    {
        variableAndPayload[flags] |= WILLFLAG;

        printf("Put the level of QoS for the WILL (0 or 1): ");
        scanf("%d", &answerQoS);
        getchar();

        if (answerQoS)
            variableAndPayload[flags] |= WILLQOS1;

        int answerRetain;
        printf("Do you want a retain for your Will (0 no | 1 yes): ");
        scanf("%d", &answerRetain);
        getchar();

        if (answerRetain)
            variableAndPayload[flags] |= RETAIN;
    }
    else{
        variableAndPayload[flags] = 0;
    }

    int newSession;
    printf("Do you want a new/clean session? (0 no, i have one | 1 yes): ");
    scanf("%d", &newSession);
    getchar();
    if (newSession)
        variableAndPayload[flags] |= CLEANSESSION;

    offset++;

    // KEEP ALIVE
    uint16_t keep_alive = htons(60);
    memcpy(variableAndPayload + offset, &keep_alive, 2);

    offset += 2;
    
    // ========== ID ==========
    if (newSession)
    {
        utfHandle(variableAndPayload, "Create your ID: ", &offset);
    }
    else
    {
        utfHandle(variableAndPayload, "Session ID: ", &offset);
    }

    // ========== WILL ==========
    if(answerWill)
    {
        utfHandle(variableAndPayload, "will topic: ", &offset);

        utfHandle(variableAndPayload, "will message: ", &offset);
    }
    else
    {
        uint32_t i = 0;
        memcpy(variableAndPayload + offset, &i, 4);
        offset += 4;
    }

    // ========== USERNAME AND PASSWORD ==========
    
    // in our case, we will always send a username and password, even in the case of a new session
    // in the case of a new session, the broker will create a new session with the username and password
    variableAndPayload[flags] |= USERNAME;
    variableAndPayload[flags] |= PASSWORD;
    utfHandle(variableAndPayload, "username: ", &offset);
    utfHandle(variableAndPayload, "password: ", &offset);
    
    int remainingLengthSize = encodeRemainingLength(offset, message + 1);

    memcpy(message + 1 + remainingLengthSize, variableAndPayload, offset);
    
    for (size_t i = 0; i < offset; i++)
    {
        printf("%02X ", (unsigned char)message[i]); // Cast char to unsigned char for correct output
    }
    printf("\n\n");
    return answerQoS;
}

int handleConnack(char *connack)
{
    switch (connack[3])
    {
    case ACCEPTED :
        printf("Connection accepted\n");
        return 1;
    case REFUSED_VERSION :
        printf("Connection refused: Unacceptable protocol version\n");
        return 0;
    case REFUSED_IDENTIFIER:
        printf("Connection refused: Unacceptable Identifier  \n");
        return 0;
    case REFUSED_SERVER_DOWN:
        printf("Connection refused: Server going down\n");
        return 0;
    case REFUSED_WRONG_USER_PASS:
        printf("Connection refused: Unacceptable username or password\n");
        return 0;
    case REFUSED_NOT_AUTHORIZED:
        printf("Connection refused: The Client's provided Client_Identifier is not allowed by the server.\n");
        return 0;   
    default:
        perror("Connection refused: Unknown error\n");
        break;
    }
    for (size_t i = 0; i < 4; i++) // 4 bytes. 4 bytes is the size of the connack message
    {
        printf("%02X ", (unsigned char)connack[i]); // Cast char to unsigned char for correct output
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
    char variableAndPayload[500];

    int qos = handleFixHeader(message, PUBLISH);

    uint16_t id;
    printf("Select a packet identifier: ");
    scanf("%d", &id);
    getchar();

    int offset = 0;

    // variable header
    utfHandle(variableAndPayload, "Topic: ", &offset);

    uint16_t identifier = htons(id);
    memcpy(variableAndPayload + offset, &identifier, 2);

    offset += 2;

    // payload
    utfHandle(variableAndPayload, "Data: ", &offset);

    int remainingLengthSize = encodeRemainingLength(offset, message + 1);

    memcpy(message + 1 + remainingLengthSize, variableAndPayload, offset);
    
    for (size_t i = 0; i < offset; i++)
    {
        printf("%02X ", (unsigned char)message[i]); // Cast char to unsigned char for correct output
    }
}

int handlePuback(char *puback)
{
    uint16_t id;
    memcpy(puback + 2, &id, 2);
    id = ntohs(id);

    printf("PUBACK received with id: %d\n", id);
}

//================================================================================================================

/*******************************************/
/*                                         */
/*               SUBSCRIBE                 */
/*                                         */
/*******************************************/

void createSubscribe(char *message)
{
    char variableAndPayload[500];
    int resQos = handleFixHeader(message, SUBSCRIBE);

    int offset = 0;

    // variable header
    uint16_t id;
    printf("Select a packet identifier: ");
    scanf("%d", &id);
    getchar();

    uint16_t identifier = htons(id);
    memcpy(variableAndPayload + offset, &identifier, 2);
    offset += 2;

    // payload
    utfHandle(variableAndPayload, "Topic to subscribe: ", &offset);

    int qos;
    printf("Put the level of QoS (0 | 1): ");
    scanf("%d", &qos);
    getchar();

    memcpy(variableAndPayload + offset, &qos, 1);
    offset += 1;

    int answ;
    printf("Do you want to subscribe to more topics (0 no | 1 yes): ");
    scanf("%d", &answ);
    getchar();
    int countertemp;

    while (answ)
    {
        utfHandle(variableAndPayload, "Topic to subscribe: ", &offset);

        printf("Put the level of QoS (0 | 1): ");
        scanf("%d", &qos);
        getchar();

        memcpy(variableAndPayload + offset, &qos, 1);
        offset += 1;

        printf("Do you want to subscribe to more topics (0 no | 1 yes): ");
        scanf("%d", &answ);
        getchar();
        countertemp += 1;
    }

    uint16_t finalSub = 0;
    memcpy(variableAndPayload + offset, &finalSub, 2);

    offset += 2;

    int remainingLengthSize = encodeRemainingLength(offset, message + 1);
    
    memcpy(message + 1 + remainingLengthSize, variableAndPayload, offset);

    for (size_t i = 0; i < offset; i++)
    {
        printf("%02X ", (unsigned char)message[i]); // Cast char to unsigned char for correct output
    }
    counter = countertemp;
}

int handleSuback(char *suback)
{
    uint16_t identifier;
    memcpy(suback + 2, &identifier, 2);
    identifier = ntohs(identifier);

    //WHILE
    for (size_t i = 0; i < counter; i++)
    {
        if ((suback[4 + i] & 0b10000000) == 0)
        {
            printf("subscription accepted");
        }
        else
        {
            printf("subscription not accepted");
        }
    }    
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

int handleRecv(void * sockfd){

    int brokerSockfd = *(int*)sockfd;

    char buf[500];

    while(1)
    {
        recv(brokerSockfd, buf, 500, 0);

        switch ((buf[0] & 0b11110000))
        {
        case PUBACK:
            handlePuback(buf);
            break;
        case SUBACK:
            handleSuback(buf);
            break;
        case PUBLISH:
            handlePublish(buf);
            break;
        default:
            break;
        }
    }
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

void handlePublish(char *args)
{
    int offset = 1;

    fixedHeader header;
    header.remainingLenght = decodeRemainingLength(args + 1);

    offset += remainingOffset(header.remainingLenght);

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

    if((variable.size !=0) && (payload.size !=0))
    {
        printf("--------------\ntopic %s updated\n", variable.topic);
        printf("[%d] %s: %s\n", variable.identifier, variable.topic, payload.data);
    }

    free(variable.topic);
    free(payload.data);
}