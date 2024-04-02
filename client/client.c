#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;

    connectVariableHeader connectVarHeader; // Initialize the instance of the variableHeader struct in the stack
    // payload

    connectPayload myconnectPayload; // Initialize the instance of the payload struct in the stack

    int sockfd;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if (getaddrinfo(NULL, MY_PORT, &hints, &res) != 0)
    {
        perror("GET ADDRES INFO");
        return 1;
    }

    // define the socket file descriptor to listen for incomming connections

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd == -1)
    {
        perror("SOCKET");
        return 1;
    }

    // bind the socketfd to the port

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0)
    {
        perror("BINDING");
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("CONNECTING");
        return 1;
    }

    printf("conection accecpted\n");

    char answer[20];
    int len = 1000;
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

    printf("select: connect, publish, subscribe, disconnect, exit\n");
    scanf("%s", answer);
    printf("You selected: %s\n", answer);

    for (;;)
    {

        if (strcmp(answer, "connect") == 0)
        {
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
                printf("ID:");
                scanf("%s", uniqueID);
                len_uniqueID = strlen(uniqueID);
                getchar();                                   // Consume the newline character that´s still in the buffer from the previous scanf
                strcpy(&message[10], (char *)&len_uniqueID); // Concatenate the length to the message
                strcat(&message[10], uniqueID);              // Concatenate the uniqueID to the message
                printf("The length of the ID is: %zu\n", strlen(uniqueID));
                // printf("The message is: %s\n", &message[10]); // Testing

                printf("Do you want to leave the will flag?:");
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
                        // free(topic);
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
                        // free(messagewill);
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
                    break;
                }
                else
                {
                    // that is the section that the server must discard any previous session information
                    strcat(&message[10], "00");
                }
                break;
            }
            // send the connect message to the server
            send(sockfd, message, strlen(message), 0);
            printf("connect message sent\n");
            free(topic);
            free(messagewill);
        }

        else if (answer == "publish")
        {
            // send the publish message to the server
            char *publishMessage = "PUBLISH";
            send(sockfd, publishMessage, strlen(publishMessage), 0);
            printf("publish message sent\n");
        }
        else if (answer == "subscribe")
        {
            // send the subscribe message to the server
            char *subscribeMessage = "SUBSCRIBE";
            send(sockfd, subscribeMessage, strlen(subscribeMessage), 0);
            printf("subscribe message sent\n");
        }
        else if (answer == "disconnect")
        {
            // send the disconnect message to the server
            char *disconnectMessage = "DISCONNECT";
            send(sockfd, disconnectMessage, strlen(disconnectMessage), 0);
            printf("disconnect message sent\n");
        }
        else if (answer == "exit")
        {
            // send the exit message to the server
            char *exitMessage = "EXIT";
            send(sockfd, exitMessage, strlen(exitMessage), 0);
            printf("exit message sent\n");
            break;
        }
    }
    free(topic);
    free(messagewill);

    close(sockfd);

    freeaddrinfo(res);

    return 0;
}