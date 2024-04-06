#include "mqttClient.h"

/*******************************************/
/*                                         */
/*                 CONNECT                 */
/*                                         */
/*******************************************/

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

void createConnectPayload(char *message)
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