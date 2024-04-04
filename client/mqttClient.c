#include "mqttClient.h"

void readConnectPayload(int sockfd, char *message)
{
    // FIXED HEADER
    message[0] = CONNECT;
    message[1] = 1;
    message[2] = 10;

    // VARIABLE HEADER
    message[3] = 1;
    message[4] = 4;
    message[5] = 'M';
    message[6] = 'Q';
    message[7] = 'T';
    message[8] = 'T';
    message[9] = 4;
    message[10] = 4;
    message[11] = 4;
    message[12] = 4;

    char asnwer[50];
    
    printf("Do you have a created session (0 no | 1 yes): ");
    scanf("%s", asnwer);

    if (strcmp(asnwer, "0") == 0)
    {
        char uniqueID[23];
        uint16_t len_uniqueID;

        printf("ID: ");
        scanf("%s", uniqueID);

        len_uniqueID = strlen(uniqueID);
        getchar();

        strcpy(&message[13], (char *)&len_uniqueID);
        strcat(&message[13], uniqueID);
        printf("The length of the ID is: %zu\n", strlen(uniqueID));

        char answerWill[20];
        printf("Do you want to leave the will flag (0 no | 1 yes): ");
        scanf("%s", answerWill);
        size_t longitud = 0;
        if (strcmp(answerWill, "1") == 0)
        {
            ssize_t readtopic;
            char *topic;

            printf("Will topic:");
            getchar();
            readtopic = getline(&topic, &longitud, stdin);

            if (readtopic != -1)
            {
                uint16_t len_topic;
                printf("You entered: %s", topic);
                printf("The length of the message is: %zu\n", strlen(topic));
                topic[strcspn(topic, "\n")] = 0;
                len_topic = longitud;
                longitud = 0;

                strcat(&message[13], (char *)&len_topic);
                strcat(&message[13], topic);
                free(topic);
            }

            char *messagewill;
            ssize_t readmessage;
            uint16_t len_message;
            longitud = 0;

            printf("Will message:");
            readmessage = getline(&messagewill, &longitud, stdin);

            if (readmessage != -1)
            {
                printf("You entered: %s", messagewill);
                printf("The length of the message is: %zu\n", strlen(messagewill));
                messagewill[strcspn(messagewill, "\n")] = 0;
                len_message = longitud;

                strcat(&message[13], (char *)&len_message);
                strcat(&message[13], messagewill);
                free(messagewill);
            }
        }
        else
        {
            strcat(&message[13], "0000");
        }
        char *username;
        ssize_t readuser;
        uint16_t len_user;
        longitud = 0;

        printf("Username:");
        readuser = getline(&username, &longitud, stdin);

        if (readuser != -1)
        {
            printf("You entered the username: %s", username);
            printf("The length of the username is: %zu\n", strlen(username) - 1);
            username[strcspn(username, "\n")] = 0;
            len_user = longitud;

            strcat(&message[13], (char *)&len_user);
            strcat(&message[13], username);
            free(username);
        }

        char *password = NULL;
        ssize_t readpassword;
        uint16_t len_pass;
        longitud = 0;

        printf("Password:");
        readpassword = getline(&password, &longitud, stdin);

        if (readpassword != -1)
        {
            printf("You entered the password: %s", password);
            printf("The length of the password is: %zu\n", strlen(password));
            password[strcspn(password, "\n")] = 0;
            len_pass = longitud;

            strcat(&message[13], (char *)&len_pass);
            strcat(&message[13], password);
            free(password);
        }
        printf("The message is: %s", message);
    }
}
int connectSocket(char *ip, char *port, int queue)
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
        close(sockfd);
        close(clientfd);
        return -1;
    }

    printf("ip conection from %s with port %s\n", hostName, serviceName);
}