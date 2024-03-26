#include "mqttServer.h"

int main(int argc, char *argv[])
{
    // create the socket file descriptor with the ip, port and queue size
    int sockfd = createSocket(NULL, MY_PORT, QUEUESIZE);

    if (sockfd == -1)
    {
        close(sockfd);
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
    
    // prints the info of the server socket and the client socket
    printSocketInfo(sockfd, clientfd, &their_addr, addr_size);

    // print the info recieved by the client 
    char buf[1000];
    int len = 1000; 

    while(buf[0] != 'q')
    {
        recv(clientfd, buf, len, 0);

        printf("info del cliente:\n\n%s\n\n", buf);

        if(buf[0] != 'q')
        {
            memset(&buf, 0, len);
        }
    }
    memset(&buf, 0, len);
    
    close(sockfd);
    close(clientfd);

    return 0;
}