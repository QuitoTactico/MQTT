#include "mqttServer.h"

mtx_t mutex;

//================================================================================================================

/*******************************************/
/*                                         */
/*                 SOCKET                  */
/*                                         */
/*******************************************/

int createSocket(char* ip, char* port, int queue)
{
    struct addrinfo hints, *res;

    // first, load up the address structs with getaddrinfo():

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4 
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

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

int printSocketInfo(int sockfd, int clientfd, struct sockaddr_storage* their_addr, socklen_t addr_size)
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

int handleRequest(void *args)
{

    SOCKET *sock = (SOCKET *)args;


    
    mtx_lock(&mutex);

    

    mtx_unlock(&mutex);

    return 0;
}