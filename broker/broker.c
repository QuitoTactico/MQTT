#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include "mqttServer.h"

int main(int argc, char *argv[])
{
    int sockfd; //socket file descriptor
    struct addrinfo hints, *res;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me
    getaddrinfo(NULL, "3490", &hints, &res);

    // make a socket:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    // bind it to the port we passed in to getaddrinfo():
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    return 0;
}