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

    // define the size of the queue to listen for incomming connections

    if (listen(sockfd, QUEUESIZE) != 0)
    {
        perror("LISTENING");
        return 1;
    }

    // accept one connection and write the info of the connected client and create its file descriptor

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    int clientfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (clientfd == -1)
    {
        perror("ACCEPT");
        return 1;
    }

    printf("conection accecpted\n\n");

    const int NAMESIZE = 100;
    char hostName[NAMESIZE];
    memset(&hostName, 0, NAMESIZE);
    char serviceName[NAMESIZE];
    memset(&serviceName, 0, NAMESIZE);
    int flags = NI_NUMERICHOST + NI_NUMERICSERV;

    if (getnameinfo((struct sockaddr *)&their_addr, addr_size, hostName, NAMESIZE, serviceName, NAMESIZE, flags) != 0)
    {
        perror("GET NAME INFO");
        return 1;
    }

    printf("ip conection from %s to %s\n", hostName, serviceName);

    // print the info recieved by the client 

    char buf[1000];
    int len = 1000;

    while(buf[0] != 'q')
    {
        recv(clientfd, buf, len, 0);

        printf("info del cliente:\n\n%s\n\n", buf);
    }

    

    close(sockfd);
    close(clientfd);

    freeaddrinfo(res);

    return 0;
}