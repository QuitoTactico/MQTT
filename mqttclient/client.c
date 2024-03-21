#include "mqttServer.h"

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
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

    /*
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("CONNECTING");
        return 1;
    }
    */

    if (listen(sockfd, QUEUESIZE) != 0)
    {
        perror("LISTENING");
        return 1;
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    int newSocketfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (newSocketfd == -1)
    {
        perror("ACCEPT");
        return 1;
    }

    printf("conection accecpted\n");

    /*
    char buf[1000];
    int len = 1000;

    recv(newSocketfd, buf, len, 0);
    */

    close(sockfd);
    close(newSocketfd);

    freeaddrinfo(res);

    return 0;
}