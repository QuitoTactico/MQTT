#include "mqttClient.h"

void readConnectPayload()
{
    char answer[20];
    char message[500];
    char anssession[50];

    char uniqueID[23];
    uint16_t len_uniqueID;

    char answerWill[20];
    // undefined lengths for the topic, message and password and user
    uint16_t len_topic; // Pointer to hold the length as a string
    char *topic = NULL;
    size_t longitud = 0;
    ssize_t readtopic;

    char *messagewill = NULL;
    ssize_t readmessage;
    uint16_t len_message;

    char *password = NULL;
    ssize_t readpassword;
    uint16_t len_pass;

    char *username = NULL;
    ssize_t readuser;
    uint16_t len_user;

    // In the first three bytes there´s reserved space for the fixed header
    message[3] = 0;
    message[4] = 4;
    message[5] = 'M';
    message[6] = 'Q';
    message[7] = 'T';
    message[8] = 'T';
    message[9] = 4;
    printf("Do you have a created session?: ");
    scanf("%s", anssession);

    // the user dont have a session, so, the server must create one and then, store it in the server to the next time the user connects
    if (strcmp(anssession, "0") == 0)
    {
        printf("ID: ");
        scanf("%s", uniqueID);
        len_uniqueID = strlen(uniqueID);
        getchar();                                   // Consume the newline character that´s still in the buffer from the previous scanf
        strcpy(&message[10], (char *)&len_uniqueID); // Concatenate the length to the message
        strcat(&message[10], uniqueID);              // Concatenate the uniqueID to the message
        printf("The length of the ID is: %zu\n", strlen(uniqueID));
        // printf("The message is: %s\n", &message[10]); // Testing

        printf("Do you want to leave the will flag: ");
        scanf("%s", answerWill);
        if (strcmp(answerWill, "0") == 0)
        {
            printf("Will topic:");
            getchar(); // Consume the newline character that´s still in the buffer from the previous scanf
            readtopic = getline(&topic, &longitud, stdin);

            if (readtopic != -1)
            {
                printf("You entered: %s", topic);
                printf("The length of the message is: %zu\n", strlen(topic));
                topic[strcspn(topic, "\n")] = 0; // Remove the newline character from the topic
                len_topic = longitud;            // Pointer to hold the length as a string
                // sprintf(len_topic, "%zu", strlen(topic));               // Convert the length to a string
                strcat(&message[10], (char *)&len_topic); // Concatenate the length to the message
                strcat(&message[10], topic);              // Concatenate the topic to the message
                free(topic);
            }
            printf("Will message:");
            readmessage = getline(&messagewill, &longitud, stdin); // the getline functions expects a pointer to a char pointer and a pointer to a size_t variable, that´s the reason why we use the & operator

            if (readmessage != -1)
            {
                printf("You entered: %s", messagewill);
                printf("The length of the message is: %zu\n", strlen(messagewill));
                messagewill[strcspn(messagewill, "\n")] = 0;
                len_message = longitud;
                strcat(&message[10], (char *)&len_message); // Concatenate the length to the message
                strcat(&message[10], messagewill);          // Concatenate the message to the message
                free(messagewill);
            }

            printf("Username:");
            readuser = getline(&username, &longitud, stdin); // Read the password

            if (readuser != -1)
            {
                printf("You entered the username: %s", username);
                printf("The length of the username is: %zu\n", strlen(username) - 1);
                username[strcspn(username, "\n")] = 0;
                len_user = longitud;

                strcat(&message[10], (char *)&len_user); // Concatenate the length to the message
                strcat(&message[10], username);          // Concatenate the message to the message
                free(username);
            }

            printf("Password:");
            readpassword = getline(&password, &longitud, stdin); // Read the password

            if (readpassword != -1)
            {
                printf("You entered the password: %s", password);
                printf("The length of the password is: %zu\n", strlen(password));
                password[strcspn(password, "\n")] = 0; // Remove the newline character from the password
                len_pass = longitud;

                strcat(&message[10], (char *)&len_pass); // Concatenate the length to the message
                strcat(&message[10], password);          // Concatenate the message to the message
                free(password);
            }
            // concatenate the topic and the message to the message variable
            printf("The message is: %s", &message[10]); // Testing
        }
        else
        {
            // that is the section that the server must discard any previous session information
            strcat(&message[10], "00");
        }
    }
}
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

    struct sockaddr *client_ptr = (struct sockaddr *)their_addr;
    char client_ip[INET6_ADDRSTRLEN];

    inet_ntop(their_addr->ss_family, &((struct sockaddr_in *)client_ptr)->sin_addr, client_ip, sizeof(client_ip));

    if (getnameinfo((struct sockaddr *)their_addr, addr_size, hostName, NAMESIZE, serviceName, NAMESIZE, flags) == -1)
    {
        perror("GET NAME INFO");
        close(sockfd);
        close(clientfd);
        return -1;
    }

    printf("ip conection from %s to %s\n", hostName, client_ip);
}