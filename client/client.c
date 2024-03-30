#include "mqttServer.h"
    #include <stdlib.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;


    struct connectVariableHeader* connectVarHeader = (connectVariableHeader*)malloc(sizeof(struct connectVariableHeader)); // Allocate memory using malloc and cast the result to the appropriate type
    //payload

    struct connectPayload* connectPayload = (struct connectPayload*)malloc(sizeof(struct connectPayload)); // Allocate memory using malloc and cast the result to the appropriate type;

    int sockfd;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // use IPv4 
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    if (getaddrinfo(NULL, MYPORT, &hints, &res) != 0)
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

    // for cycle to ask the user the action that wants to do, like: select: connect, publish, subscribe, etc.
    char answer[1000];
    int len = 1000;

    printf("select: connect, publish, subscribe, disconnect, exit\n");
    scanf("%s", answer);
    for(;;){
        if(answer == "connect"){
            //Work with the instance of the variableHeader struct

            connectVarHeader->connectFlags = 1;



            // send the connect message to the server
            char* connectMessage = "CONNECT";
            send(sockfd, connectMessage, strlen(connectMessage), 0);
            printf("connect message sent\n");
        }
        else if(answer == "publish"){
            // send the publish message to the server
            char* publishMessage = "PUBLISH";
            send(sockfd, publishMessage, strlen(publishMessage), 0);
            printf("publish message sent\n");
        }
        else if(answer == "subscribe"){
            // send the subscribe message to the server
            char* subscribeMessage = "SUBSCRIBE";
            send(sockfd, subscribeMessage, strlen(subscribeMessage), 0);
            printf("subscribe message sent\n");
        }
        else if(answer == "disconnect"){
            // send the disconnect message to the server
            char* disconnectMessage = "DISCONNECT";
            send(sockfd, disconnectMessage, strlen(disconnectMessage), 0);
            printf("disconnect message sent\n");
        }
        else if(answer == "exit"){
            // send the exit message to the server
            char* exitMessage = "EXIT";
            send(sockfd, exitMessage, strlen(exitMessage), 0);
            printf("exit message sent\n");
            break;



        }



    }






    close(sockfd);

    freeaddrinfo(res);

    return 0;
}